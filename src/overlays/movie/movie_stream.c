/**
 * @file movie_stream.c
 * @brief CD ring buffers and the MDEC/GPU output pipeline.
 *
 * Each frame reserves consecutive sectors. If it cannot fit at the end of a
 * ring, the producer records that end as the wrap index and starts at zero.
 * Equal read/write indices mean empty only when the last-frame markers agree.
 * Video payloads omit their headers so VLC can read a contiguous bitstream;
 * audio sectors retain their headers for the AKAO stream player.
 */
#include "movie_internal.h"

/* The CD streaming callback tests this token for NULL; it never dereferences it. */
#define MOVIE_STREAM_CONTINUE ((u8*)1)

static void movie_schedule_next_decode(void);

/**
 * @brief Upload completed MDEC output and schedule the next decode.
 * @see https://decomp.me/scratch/HVkZ6 (100%)
 */
void movie_mdec_out_callback(void)
{
    MovieState* state = MOVIE_STATE;
    s32 draw_status;
    u_long* ordering_table;

    /* Queue a standard GPU upload or defer it while drawing is busy. */
    if (g_gpu_mode == MOVIE_GPU_MODE_STANDARD)
    {
        if (g_cd_data_ready_pending == CD_READY_CALLBACK_PENDING)
        {
            cdrom_verify_recovery();
        }
        draw_status = DrawSync(DRAW_SYNC_MODE_POLL);
        if (draw_status < DRAW_SYNC_DEFER_THRESHOLD)
        {
            LoadImage(&state->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
            state->draw_sync_target = draw_status + 1;
        }
        else
        {
            state->pending_vram_upload = TRUE;
        }
    }
    else
    {
        /* Suspend drawing around the alternate transfer path. */
        ordering_table = BreakDraw();
        if (ordering_table != BREAK_DRAW_FAILURE)
        {
            LoadImage2(&state->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
            if (ordering_table != NULL)
            {
                DrawOTag(ordering_table);
            }
        }
        else
        {
            LoadImage(&state->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
        }
    }

    /* Continue decoding unless the upload was deferred. */
    if (!MOVIE_STATE->pending_vram_upload)
    {
        movie_schedule_next_decode();
        return;
    }
    MOVIE_STATE->mdec_busy = MDEC_STATE_ACTIVE;
}

/**
 * @brief Advance the output slice and schedule the next MDEC transfer.
 * @see https://decomp.me/scratch/E7XCZ (100%)
 */
static void movie_schedule_next_decode(void)
{
    u16 next_output_buffer;
    u16 current_x;
    s16 slice_width;
    s16 next_x;
    s32 slice_x;
    s32 chunk_end_x;
    s32 pixel_count;

    /* Advance horizontally and alternate the MDEC output buffer. */
    next_output_buffer = 1 - MOVIE_STATE->out_buf_idx;
    current_x = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x;
    slice_width = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w;
    next_x = current_x + slice_width;
    MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x = next_x;
    slice_x = next_x;
    MOVIE_STATE->out_buf_idx = next_output_buffer;

    chunk_end_x = MOVIE_STATE->rects[MOVIE_STATE->chunk_idx].x + MOVIE_STATE->rects[MOVIE_STATE->chunk_idx].w;

    if (slice_x < chunk_end_x)
    {
        /* Decode the next slice now, or defer until the GPU queue drains. */
        if (MOVIE_STATE->draw_sync_target < DRAW_SYNC_DEFER_THRESHOLD)
        {
            pixel_count = slice_width * MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h;
            DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], pixel_count / 2);
            MOVIE_STATE->mdec_busy = MDEC_STATE_CHAINED;
        }
        else
        {
            MOVIE_STATE->mdec_busy = MDEC_STATE_ACTIVE;
            MOVIE_STATE->pending_mdec_decode = TRUE;
        }
    }
    else
    {
        /* Publish the completed frame and start the other display buffer. */
        MOVIE_STATE->chunk_idx = 1 - MOVIE_STATE->chunk_idx;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x = MOVIE_STATE->rects[MOVIE_STATE->chunk_idx].x;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].y = MOVIE_STATE->rects[MOVIE_STATE->chunk_idx].y;
        MOVIE_STATE->frame_ready = TRUE;
        MOVIE_STATE->mdec_busy = MDEC_STATE_IDLE;
        if (MOVIE_STATE->end_state == END_STATE_NEAR_END)
        {
            MOVIE_STATE->end_state = END_STATE_DONE;
        }
    }
}

/**
 * @brief Service deferred MDEC output and VRAM uploads.
 * @see https://decomp.me/scratch/JTTFr (100%)
 */
void movie_service_video_ops(void)
{
    u_long* ordering_table;

    if (!MOVIE_STATE->pending_vram_upload && !MOVIE_STATE->pending_mdec_decode)
    {
        return;
    }

    /* Service queued work through the synchronized GPU transfer path. */
    if (MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD)
    {
        if (DrawSync(DRAW_SYNC_MODE_POLL) >= DRAW_SYNC_DEFER_THRESHOLD)
        {
            return;
        }
        if (MOVIE_STATE->pending_vram_upload)
        {
            MOVIE_STATE->video_service_busy = TRUE;
            /* Recheck the request after claiming the busy flag; a callback may have serviced it. */
            if (MOVIE_STATE->pending_vram_upload)
            {
                LoadImage(&MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX], MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx]);
                MOVIE_STATE->draw_sync_target = DrawSync(DRAW_SYNC_MODE_POLL) + 1;
                MOVIE_STATE->pending_vram_upload = FALSE;
                movie_schedule_next_decode();
            }
            MOVIE_STATE->video_service_busy = FALSE;
        }

        /* Submit a deferred MDEC output transfer. */
        if (MOVIE_STATE->pending_mdec_decode)
        {
            MOVIE_STATE->video_service_busy = TRUE;
            if (MOVIE_STATE->pending_mdec_decode)
            {
                s32 pixel_count;

                pixel_count = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w * MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h;
                DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], pixel_count / 2);
                MOVIE_STATE->pending_mdec_decode = FALSE;
            }
            MOVIE_STATE->video_service_busy = FALSE;
        }
    }
    else
    {
        /* Interrupt drawing to service a pending upload immediately. */
        if (MOVIE_STATE->pending_vram_upload)
        {
            MOVIE_STATE->video_service_busy = TRUE;
            if (MOVIE_STATE->pending_vram_upload)
            {
                ordering_table = BreakDraw();
                if (ordering_table != BREAK_DRAW_FAILURE)
                {
                    LoadImage2(&MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX], MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx]);
                    if (ordering_table != NULL)
                    {
                        DrawOTag(ordering_table);
                    }
                    movie_schedule_next_decode();
                    MOVIE_STATE->pending_vram_upload = FALSE;
                }
            }
            g_movie_video_service_busy = FALSE;
        }
    }
}

/**
 * @brief Buffer an arriving movie sector in the video or audio ring.
 *
 * Reads the 32-byte stream header and tracks multi-sector frame continuations.
 * @param bytes_transferred Unused; the movie header supplies the frame position.
 * @param bytes_remaining Unused; playback stops at the configured last frame.
 * @return A non-NULL continuation token, or NULL to stop the CD read.
 * @see https://decomp.me/scratch/5flHR (100%)
 */
u8* cd_sector_callback(s32 bytes_transferred, u32 bytes_remaining)
{
    VideoSectorEntry sector_header;
    s32 audio_read_idx;
    s32 ring_has_room;
    u8* continuation_payload;
    SectorEntry* continuation_header;
    s32 next_audio_write_idx;
    s32 video_write_idx;
    s32 video_read_idx;

    ring_has_room = FALSE;

    /* Read and classify the first sector of a frame chunk. */
    if (MOVIE_STATE->sectors_remaining == 0)
    {
        MovieState* state;
        u32* header_words;
        while (CdGetSector(&sector_header, CD_HEADER_WORDS) == 0)
        {
        }

        state = MOVIE_STATE;
        if (sector_header.header.frame_number > MOVIE_STATE->total_frames)
        {
            MOVIE_STATE->end_of_stream = TRUE;
            return NULL;
        }

        state->frame_number = sector_header.header.frame_number;

        if (sector_header.header.chunk_sector_idx != 0)
        {
            return MOVIE_STREAM_CONTINUE;
        }

        if (sector_header.header.sector_type == SECTOR_TYPE_VIDEO)
        {
            /* Reserve contiguous space in the video ring. */
            video_write_idx = state->video_write_idx;
            video_read_idx = state->video_read_idx;

            if (((video_write_idx == video_read_idx) && (state->last_video_frame == state->last_consumed_video_frame)) ||
                ((video_write_idx != video_read_idx) && (video_read_idx < state->video_write_idx)))
            {
                if (state->video_ring_capacity < (state->video_write_idx + sector_header.header.sector_count))
                {
                    if (video_read_idx >= sector_header.header.sector_count)
                    {
                        ring_has_room = TRUE;
                        state->video_wrap_idx = state->video_write_idx;
                        state->video_write_idx = 0;
                    }
                }
                else
                {
                    ring_has_room = TRUE;
                }
            }
            else if (video_write_idx != video_read_idx)
            {
                if (video_read_idx >= (state->video_write_idx + sector_header.header.sector_count))
                {
                    ring_has_room = TRUE;
                }
            }

            if (ring_has_room)
            {
                s32 write_index;
                u32* sector;

                /* Read the payload and retain its raw stream header. */
                sector = (u32*)MOVIE_STATE->video_data_base[MOVIE_STATE->video_write_idx].data;
                while (CdGetSector(sector, CD_PAYLOAD_WORDS) == 0)
                {
                }

                header_words = sector_header.words;

                write_index = MOVIE_STATE->video_write_idx;
                sector = MOVIE_STATE->video_table_base[write_index].words;

                sector[0] = header_words[0];
                sector[1] = header_words[1];
                sector[2] = header_words[2];
                sector[3] = header_words[3];
                sector[4] = header_words[4];
                sector[5] = header_words[5];
                sector[6] = header_words[6];
                sector[7] = header_words[7];

                MOVIE_STATE->sectors_remaining = sector_header.header.sector_count - 1;
                if (MOVIE_STATE->sectors_remaining == 0)
                {
                    u32 total_frames;

                    total_frames = MOVIE_STATE->total_frames;
                    MOVIE_STATE->video_write_idx = MOVIE_STATE->video_write_idx + 1;

                    MOVIE_STATE->last_video_frame = MOVIE_STATE->frame_number;

                    if (MOVIE_STATE->frame_number >= total_frames)
                    {
                        return NULL;
                    }
                }
                else
                {
                    MOVIE_STATE->continuation_type = CONTINUATION_VIDEO;
                    MOVIE_STATE->chunk_sector_idx = 1;
                }
            }
        }
        else
        {
            MovieState* movie_state;
            s32 audio_write_idx;

            /* Reserve contiguous space in the audio ring. */
            audio_write_idx = state->audio_write_idx;
            audio_read_idx = state->audio_read_idx;

            if (((audio_write_idx == audio_read_idx) && (state->last_audio_frame == state->last_consumed_audio_frame)) ||
                ((audio_write_idx != audio_read_idx) && (audio_read_idx < state->audio_write_idx)))
            {
                if (state->audio_ring_capacity < (state->audio_write_idx + sector_header.header.sector_count))
                {
                    if (audio_read_idx >= sector_header.header.sector_count)
                    {
                        ring_has_room = TRUE;
                        state->audio_wrap_idx = state->audio_write_idx;
                        state->audio_write_idx = 0;
                    }
                }
                else
                {
                    ring_has_room = TRUE;
                }
            }
            else if ((audio_write_idx != audio_read_idx) && (audio_read_idx >= (state->audio_write_idx + sector_header.header.sector_count)))
            {
                ring_has_room = TRUE;
            }

            if (ring_has_room)
            {
                u32* sector;

                /* Read the payload and retain its raw stream header. */
                sector = (u32*)MOVIE_STATE->audio_data_base[MOVIE_STATE->audio_write_idx].payload;
                while (CdGetSector(sector, CD_PAYLOAD_WORDS) == 0)
                {
                }

                header_words = sector_header.words;
                sector = MOVIE_STATE->audio_data_base[MOVIE_STATE->audio_write_idx].header_block.words;
                sector[0] = header_words[0];
                sector[1] = header_words[1];
                sector[2] = header_words[2];
                sector[3] = header_words[3];
                sector[4] = header_words[4];
                sector[5] = header_words[5];
                sector[6] = header_words[6];
                sector[7] = header_words[7];
                MOVIE_STATE->sectors_remaining = sector_header.header.sector_count - 1;
                if (MOVIE_STATE->sectors_remaining == 0)
                {
                    MOVIE_STATE->audio_write_idx = MOVIE_STATE->audio_write_idx + 1;
                    MOVIE_STATE->last_audio_frame = MOVIE_STATE->frame_number;

                    if (MOVIE_STATE->frame_number > MOVIE_STATE->total_frames)
                    {
                        return NULL;
                    }
                }
                else
                {
                    MOVIE_STATE->continuation_type = CONTINUATION_AUDIO;
                    MOVIE_STATE->chunk_sector_idx = 1;
                }
            }
            movie_state = MOVIE_STATE;
            if (g_movie_audio_stream_state == AUDIO_STREAM_STATE_SECTOR_READY)
            {
                movie_state->audio_stream_state = AUDIO_STREAM_STATE_PRIMED;
            }
            return MOVIE_STREAM_CONTINUE;
        }
    }
    else if (MOVIE_STATE->continuation_type == CONTINUATION_VIDEO)
    {
        /* Validate and append a video continuation sector. */
        continuation_header = &MOVIE_STATE->video_table_base[MOVIE_STATE->video_write_idx + MOVIE_STATE->chunk_sector_idx].header;
        while (CdGetSector(continuation_header, CD_HEADER_WORDS) == 0)
        {
        }

        if (((continuation_header->sector_type == SECTOR_TYPE_VIDEO) && (continuation_header->frame_number == MOVIE_STATE->frame_number)) &&
            (continuation_header->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            continuation_payload = MOVIE_STATE->video_data_base[MOVIE_STATE->video_write_idx + MOVIE_STATE->chunk_sector_idx].data;
            while (CdGetSector(continuation_payload, CD_PAYLOAD_WORDS) == 0)
            {
            }

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (MOVIE_STATE->sectors_remaining == 0)
            {
                u32 total_frames;
                total_frames = MOVIE_STATE->total_frames;

                MOVIE_STATE->video_write_idx += 1 + MOVIE_STATE->chunk_sector_idx;

                MOVIE_STATE->last_video_frame = MOVIE_STATE->frame_number;
                if (continuation_header->frame_number >= total_frames)
                {
                    return NULL;
                }
            }
            else
            {
                MOVIE_STATE->chunk_sector_idx = MOVIE_STATE->chunk_sector_idx + 1;
            }
        }
        else
        {
            MOVIE_STATE->frame_number = continuation_header->frame_number;
            MOVIE_STATE->sectors_remaining = 0U;

            if (MOVIE_STATE->total_frames > continuation_header->frame_number)
            {
                return MOVIE_STREAM_CONTINUE;
            }

            MOVIE_STATE->end_of_stream = TRUE;
            return NULL;
        }
    }
    else
    {
        /* Validate and append an audio continuation sector. */
        continuation_header = &MOVIE_STATE->audio_data_base[MOVIE_STATE->audio_write_idx + MOVIE_STATE->chunk_sector_idx].header_block.header;
        while (CdGetSector(continuation_header, CD_HEADER_WORDS) == 0)
        {
        }

        if (((continuation_header->sector_type == SECTOR_TYPE_AUDIO) && (continuation_header->frame_number == MOVIE_STATE->frame_number)) &&
            (continuation_header->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            continuation_payload = MOVIE_STATE->audio_data_base[MOVIE_STATE->audio_write_idx + MOVIE_STATE->chunk_sector_idx].payload;
            while (CdGetSector(continuation_payload, CD_PAYLOAD_WORDS) == 0)
            {
            }

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (MOVIE_STATE->sectors_remaining == 0)
            {
                next_audio_write_idx = MOVIE_STATE->audio_write_idx + 1;
                MOVIE_STATE->audio_write_idx = next_audio_write_idx + MOVIE_STATE->chunk_sector_idx;

                MOVIE_STATE->last_audio_frame = MOVIE_STATE->frame_number;
                if (continuation_header->frame_number > MOVIE_STATE->total_frames)
                {
                    return NULL;
                }
            }
            else
            {
                MOVIE_STATE->chunk_sector_idx = MOVIE_STATE->chunk_sector_idx + 1;
            }
        }
        else
        {
            MOVIE_STATE->frame_number = continuation_header->frame_number;
            MOVIE_STATE->sectors_remaining = 0U;
            if (continuation_header->frame_number <= MOVIE_STATE->total_frames)
            {
                return MOVIE_STREAM_CONTINUE;
            }

            MOVIE_STATE->end_of_stream = TRUE;
            return NULL;
        }
    }

    return MOVIE_STREAM_CONTINUE;
}

/**
 * @brief Return the next audio-frame entry not yet queued for playback.
 *
 * @param out_entry Receives the first sector of the next available audio frame.
 * @return 1 if an entry is available, otherwise 0.
 * @see https://decomp.me/scratch/I2Ddr (100%)
 */
s32 get_next_audio_entry(AudioSector** out_entry)
{
    s32 next_entry_idx;
    AudioSector* next_entry;

    /* Stop when the producer and consumer identify the same completed frame. */
    if ((MOVIE_STATE->audio_write_idx == MOVIE_STATE->audio_read_idx) && (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
    {
        return 0;
    }

    /* Wrap the read cursor after consuming the previous contiguous segment. */
    if ((MOVIE_STATE->audio_write_idx <= MOVIE_STATE->audio_read_idx) && (MOVIE_STATE->audio_read_idx == MOVIE_STATE->audio_wrap_idx))
    {
        MOVIE_STATE->audio_read_idx = 0;

        if (MOVIE_STATE->audio_write_idx == 0 && (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
        {
            return 0;
        }
    }

    /* Skip entries already queued to the audio pipeline. */
    next_entry_idx = MOVIE_STATE->audio_read_idx + MOVIE_STATE->audio_buffered_count;

    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_entry_idx >= MOVIE_STATE->audio_wrap_idx))
    {
        next_entry_idx -= MOVIE_STATE->audio_wrap_idx;
    }

    if ((next_entry_idx == MOVIE_STATE->audio_write_idx) && (MOVIE_STATE->audio_buffered_count != 0))
    {
        return 0;
    }

    /* Account the returned frame's sectors as queued. */
    next_entry = &MOVIE_STATE->audio_data_base[next_entry_idx];
    MOVIE_STATE->audio_buffered_count += next_entry->header_block.header.sector_count;
    *out_entry = next_entry;
    return 1;
}

/**
 * @brief Complete deferred video work after GPU drawing becomes idle.
 * @see https://decomp.me/scratch/TApbR (100%)
 */
void draw_sync_callback(void)
{
    s32 pixel_count;
    MovieState* state = MOVIE_STATE;

    if (g_movie_video_service_busy)
    {
        return;
    }

    state->draw_sync_target = 0;

    /* Upload deferred MDEC output and advance the decode position. */
    if (state->pending_vram_upload)
    {
        LoadImage(&state->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
        movie_schedule_next_decode();
        state->pending_vram_upload = FALSE;
    }

    /* Submit a deferred MDEC output transfer. */
    if (state->pending_mdec_decode)
    {
        pixel_count = state->rects[MDEC_OUTPUT_RECT_INDEX].w * state->rects[MDEC_OUTPUT_RECT_INDEX].h;
        DecDCTout(state->mdec_output_buf[state->out_buf_idx], pixel_count / 2);
        state->pending_mdec_decode = FALSE;
    }
}

/**
 * @brief Return the payload and header for the next readable video frame.
 *
 * @param out_vlc_data Receives the frame's VLC payload.
 * @param out_entry_header Receives the frame's sector header.
 * @return 1 if an entry is available, otherwise 0.
 * @see https://decomp.me/scratch/OJvsJ (100%)
 */
s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header)
{
    s32 read_index;
    s32 write_index;

    /* Stop when the producer and consumer identify the same completed frame. */
    if ((MOVIE_STATE->video_write_idx == MOVIE_STATE->video_read_idx) && (MOVIE_STATE->last_video_frame == MOVIE_STATE->last_consumed_video_frame))
    {
        return 0;
    }

    write_index = MOVIE_STATE->video_write_idx;
    read_index = MOVIE_STATE->video_read_idx;

    /* Wrap after consuming the previous contiguous ring segment. */
    if ((read_index >= write_index) && (read_index == MOVIE_STATE->video_wrap_idx))
    {
        MOVIE_STATE->video_read_idx = 0;

        if ((MOVIE_STATE->video_write_idx == 0) && (MOVIE_STATE->last_video_frame == MOVIE_STATE->last_consumed_video_frame))
        {
            return 0;
        }
    }

    /* Resolve the parallel header and VLC payload slots. */
    *out_entry_header = &MOVIE_STATE->video_table_base[MOVIE_STATE->video_read_idx];
    *out_vlc_data = &MOVIE_STATE->video_data_base[MOVIE_STATE->video_read_idx];
    return 1;
}

/**
 * @brief Advance the video ring past the current frame.
 * @see https://decomp.me/scratch/SUBK5 (100%)
 */
void advance_video_read(void)
{
    const SectorEntry* frame_header;
    s32 next_read_index;

    /* Advance by the number of sectors comprising the current frame. */
    frame_header = &MOVIE_STATE->video_table_base[MOVIE_STATE->video_read_idx].header;
    next_read_index = MOVIE_STATE->video_read_idx + frame_header->sector_count;

    /* Wrap after consuming the older contiguous ring segment. */
    if ((MOVIE_STATE->video_read_idx >= MOVIE_STATE->video_write_idx) && (next_read_index == MOVIE_STATE->video_wrap_idx))
    {
        next_read_index = 0;
    }

    /* Commit the completed frame and next consumer position. */
    MOVIE_STATE->last_consumed_video_frame = frame_header->frame_number;
    MOVIE_STATE->video_read_idx = next_read_index;
}

/**
 * @brief Advance the audio ring past the current frame.
 * @see https://decomp.me/scratch/6Xjsu (100%)
 */
void advance_audio_read(void)
{
    const SectorEntry* frame_header;
    s32 next_read_index;

    /* Advance past the frame and release its queued sectors. */
    frame_header = &MOVIE_STATE->audio_data_base[MOVIE_STATE->audio_read_idx].header_block.header;
    next_read_index = MOVIE_STATE->audio_read_idx + frame_header->sector_count;
    MOVIE_STATE->audio_buffered_count -= frame_header->sector_count;

    /* This path uses the video ring's wrap point, unlike audio entry lookup. */
    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_read_index == MOVIE_STATE->video_wrap_idx))
    {
        next_read_index = 0;
    }

    /* Commit the completed frame and next consumer position. */
    MOVIE_STATE->last_consumed_audio_frame = frame_header->frame_number;
    MOVIE_STATE->audio_read_idx = next_read_index;
}
