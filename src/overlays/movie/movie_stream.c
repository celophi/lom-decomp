#include "movie_internal.h"

/**
 * @brief Upload completed MDEC output and schedule the next decode.
 * @see https://decomp.me/scratch/HVkZ6 (100%)
 */
void movie_mdec_out_callback(void)
{
    volatile MovieState* state = VOL_MOVIE_STATE;
    MovieGpuResult gpu_result;
    s32 inactive = 0;

    /* Queue a standard GPU upload or defer it while drawing is busy. */
    if (g_gpu_mode == MOVIE_GPU_MODE_STANDARD)
    {
        if (g_cd_status_byte_3 == CD_STATUS_RECOVERY_PENDING)
        {
            cdrom_verify_recovery();
        }
        gpu_result.sync_status = DrawSync(DRAW_SYNC_MODE_POLL);
        if (gpu_result.sync_status < DRAW_SYNC_DEFER_THRESHOLD)
        {
            LoadImage(&MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
            state->draw_sync_target = gpu_result.sync_status + 1;
        }
        else
        {
            state->pending_vram_upload = TRUE;
        }
    }
    else
    {
        /* Suspend drawing around the alternate transfer path. */
        gpu_result.ordering_table = BreakDraw();
        if (gpu_result.address != BREAK_DRAW_FAILURE_ADDRESS)
        {
            LoadImage2(&MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
            if (gpu_result.address != inactive)
            {
                DrawOTag(gpu_result.ordering_table);
            }
        }
        else
        {
            LoadImage(&MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
        }
    }

    /* Continue decoding unless the upload was deferred. */
    if (MOVIE_STATE->pending_vram_upload == inactive)
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
void movie_schedule_next_decode(void)
{
    u16 next_output_buffer;
    u16 current_x;
    MovieHalfword slice_width;
    MovieHalfword next_x;
    s32 signed_next_x;
    s32 chunk_end_x;
    s32 pixel_count;
    s32 decode_word_count;

    /* Advance horizontally and alternate the MDEC output buffer. */
    next_output_buffer = 1 - MOVIE_STATE->out_buf_idx;
    current_x = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x;
    slice_width.raw = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w;
    next_x.raw = current_x + slice_width.raw;
    MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x = next_x.raw;
    signed_next_x = next_x.signed_value;
    VOL_MOVIE_STATE->out_buf_idx = next_output_buffer;

    chunk_end_x = MOVIE_STATE->rects[VOL_MOVIE_STATE->chunk_idx].x + MOVIE_STATE->rects[VOL_MOVIE_STATE->chunk_idx].w;

    if (signed_next_x < chunk_end_x)
    {
        /* Decode the next slice now, or defer until the GPU queue drains. */
        if (MOVIE_STATE->draw_sync_target < DRAW_SYNC_DEFER_THRESHOLD)
        {
            pixel_count = slice_width.signed_value * MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h;
            decode_word_count = pixel_count + SIGNED_HALF_ROUNDING(pixel_count);
            DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], decode_word_count >> 1);
            VOL_MOVIE_STATE->mdec_busy = MDEC_STATE_CHAINED;
        }
        else
        {
            MOVIE_STATE->mdec_busy = MDEC_STATE_ACTIVE;
            VOL_MOVIE_STATE->pending_mdec_decode = TRUE;
        }
    }
    else
    {
        /* Publish the completed chunk and reset to the alternate display area. */
        MOVIE_STATE->chunk_idx = 1 - MOVIE_STATE->chunk_idx;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x = MOVIE_STATE->rects[VOL_MOVIE_STATE->chunk_idx].x;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].y = MOVIE_STATE->rects[VOL_MOVIE_STATE->chunk_idx].y;
        VOL_MOVIE_STATE->frame_ready = TRUE;
        VOL_MOVIE_STATE->mdec_busy = MDEC_STATE_IDLE;
        if (VOL_MOVIE_STATE->end_state == END_STATE_NEAR_END)
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
    volatile MovieState* state = VOL_MOVIE_STATE;
    MovieGpuResult break_draw_result;

    if (!state->pending_vram_upload && !state->pending_mdec_decode)
    {
        return;
    }

    /* Service queued work through the synchronized GPU transfer path. */
    if (VOL_MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD)
    {
        if (DrawSync(DRAW_SYNC_MODE_POLL) >= DRAW_SYNC_DEFER_THRESHOLD)
        {
            return;
        }
        if (VOL_MOVIE_STATE->pending_vram_upload)
        {
            MOVIE_STATE->busy = TRUE;
            /* Recheck the volatile request after claiming the shared busy flag. */
            if (VOL_MOVIE_STATE->pending_vram_upload)
            {
                LoadImage(&MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[VOL_MOVIE_STATE->out_buf_idx]);
                VOL_MOVIE_STATE->draw_sync_target = DrawSync(DRAW_SYNC_MODE_POLL) + 1;
                VOL_MOVIE_STATE->pending_vram_upload = FALSE;
                movie_schedule_next_decode();
            }
            VOL_MOVIE_STATE->busy = FALSE;
        }
        state = VOL_MOVIE_STATE;

        /* Submit a deferred MDEC output transfer. */
        if (state->pending_mdec_decode)
        {
            MOVIE_STATE->busy = TRUE;
            if (state->pending_mdec_decode)
            {
                s32 pixel_count;

                pixel_count = state->rects[MDEC_OUTPUT_RECT_INDEX].w * state->rects[MDEC_OUTPUT_RECT_INDEX].h;
                DecDCTout(state->mdec_output_buf[state->out_buf_idx], pixel_count / 2);
                state->pending_mdec_decode = FALSE;
            }
            MOVIE_STATE->busy = FALSE;
        }
    }
    else
    {
        /* Interrupt drawing to service a pending upload immediately. */
        if (state->pending_vram_upload)
        {
            MOVIE_STATE->busy = TRUE;
            if (state->pending_vram_upload)
            {
                break_draw_result.ordering_table = BreakDraw();
                if (break_draw_result.address != BREAK_DRAW_FAILURE_ADDRESS)
                {
                    LoadImage2(&MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX], state->mdec_output_buf[state->out_buf_idx]);
                    if (break_draw_result.ordering_table != NULL)
                    {
                        DrawOTag(break_draw_result.ordering_table);
                    }
                    movie_schedule_next_decode();
                    state->pending_vram_upload = FALSE;
                }
            }
            g_busy = FALSE;
        }
    }
}

/**
 * @brief Buffer an arriving movie sector in the video or audio ring.
 *
 * Reads the 32-byte stream header and tracks multi-sector frame continuations.
 * @return 1 while streaming should continue, otherwise 0.
 * @see https://decomp.me/scratch/5flHR (100%)
 */
s32 cd_sector_callback(void)
{
    VideoSectorEntry sector_header;
    s32 audio_read_idx;
    s32 ring_has_room;
    u32 has_more_frames;
    u8* continuation_payload;
    SectorEntry* continuation_header;
    s32 next_audio_write_idx;
    s32 video_write_idx;
    s32 video_read_idx;

    ring_has_room = FALSE;

    /* Read and classify the first sector of a frame chunk. */
    if (MOVIE_STATE->sectors_remaining == 0)
    {
        volatile MovieState* state;
        u32* header_words;
        while (CdGetSector(&sector_header, CD_HEADER_WORDS) == 0)
        {
        }

        state = VOL_MOVIE_STATE;
        if (sector_header.header.frame_number > MOVIE_STATE->total_frames)
        {
            VOL_MOVIE_STATE->end_of_stream = TRUE;
            return 0;
        }

        state->frame_number = sector_header.header.frame_number;

        if (sector_header.header.chunk_sector_idx != 0)
        {
            return 1;
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
                        state->video_ring_size = state->video_write_idx;
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
                MovieStreamEntryPointer sector;

                /* Read the payload and retain its raw stream header. */
                sector.video_payload = &MOVIE_STATE->video_data_base[MOVIE_STATE->video_write_idx];
                while (CdGetSector(sector.video_payload->data, CD_PAYLOAD_WORDS) == 0)
                {
                }

                header_words = sector_header.words;

                write_index = MOVIE_STATE->video_write_idx;
                sector.video_header = &MOVIE_STATE->video_table_base[write_index];

                sector.video_header->words[0] = header_words[0];
                sector.video_header->words[1] = header_words[1];
                sector.video_header->words[2] = header_words[2];
                sector.video_header->words[3] = header_words[3];
                sector.video_header->words[4] = header_words[4];
                sector.video_header->words[5] = header_words[5];
                sector.video_header->words[6] = header_words[6];
                sector.video_header->words[7] = header_words[7];

                MOVIE_STATE->sectors_remaining = sector_header.header.sector_count - 1;
                if (MOVIE_STATE->sectors_remaining == 0)
                {
                    u32 total_frames;

                    total_frames = MOVIE_STATE->total_frames;
                    VOL_MOVIE_STATE->video_write_idx = VOL_MOVIE_STATE->video_write_idx + 1;

                    VOL_MOVIE_STATE->last_video_frame = VOL_MOVIE_STATE->frame_number;

                    has_more_frames = VOL_MOVIE_STATE->frame_number < total_frames;

                    goto check_end_of_stream;
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
                        state->audio_ring_size = state->audio_write_idx;
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
                MovieStreamEntryPointer sector;

                /* Read the payload and retain its raw stream header. */
                sector.payload = MOVIE_STATE->audio_data_base[VOL_MOVIE_STATE->audio_write_idx].payload;
                while (CdGetSector(sector.payload, CD_PAYLOAD_WORDS) == 0)
                {
                }

                header_words = sector_header.words;
                sector.audio_sector = &MOVIE_STATE->audio_data_base[VOL_MOVIE_STATE->audio_write_idx];
                sector.audio_sector->header_block.words[0] = header_words[0];
                sector.audio_sector->header_block.words[1] = header_words[1];
                sector.audio_sector->header_block.words[2] = header_words[2];
                sector.audio_sector->header_block.words[3] = header_words[3];
                sector.audio_sector->header_block.words[4] = header_words[4];
                sector.audio_sector->header_block.words[5] = header_words[5];
                sector.audio_sector->header_block.words[6] = header_words[6];
                sector.audio_sector->header_block.words[7] = header_words[7];
                MOVIE_STATE->sectors_remaining = sector_header.header.sector_count - 1;
                if (MOVIE_STATE->sectors_remaining == 0)
                {
                    VOL_MOVIE_STATE->audio_write_idx = VOL_MOVIE_STATE->audio_write_idx + 1;
                    VOL_MOVIE_STATE->last_audio_frame = VOL_MOVIE_STATE->frame_number;

                    if (VOL_MOVIE_STATE->frame_number > MOVIE_STATE->total_frames)
                    {
                        return 0;
                    }
                }
                else
                {
                    MOVIE_STATE->continuation_type = CONTINUATION_AUDIO;
                    MOVIE_STATE->chunk_sector_idx = 1;
                }
            }
            movie_state = MOVIE_STATE;
            if (g_audioStreamState == AUDIO_STREAM_STATE_SECTOR_READY)
            {
                movie_state->audio_stream_state = AUDIO_STREAM_STATE_PRIMED;
            }
            return 1;
        }
    }
    else if (MOVIE_STATE->continuation_type == CONTINUATION_VIDEO)
    {
        /* Validate and append a video continuation sector. */
        continuation_header = &MOVIE_STATE->video_table_base[VOL_MOVIE_STATE->video_write_idx + MOVIE_STATE->chunk_sector_idx].header;
        while (CdGetSector(continuation_header, CD_HEADER_WORDS) == 0)
        {
        }

        if (((continuation_header->sector_type == SECTOR_TYPE_VIDEO) && (continuation_header->frame_number == MOVIE_STATE->frame_number)) &&
            (continuation_header->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            continuation_payload = MOVIE_STATE->video_data_base[VOL_MOVIE_STATE->video_write_idx + MOVIE_STATE->chunk_sector_idx].data;
            while (CdGetSector(continuation_payload, CD_PAYLOAD_WORDS) == 0)
            {
            }

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (MOVIE_STATE->sectors_remaining == 0)
            {
                s32 first_sector_count;
                u32 total_frames;
                total_frames = MOVIE_STATE->total_frames;

                first_sector_count = 1;
                VOL_MOVIE_STATE->video_write_idx = (VOL_MOVIE_STATE->video_write_idx + first_sector_count) + MOVIE_STATE->chunk_sector_idx;

                VOL_MOVIE_STATE->last_video_frame = VOL_MOVIE_STATE->frame_number;
                has_more_frames = continuation_header->frame_number < total_frames;

            check_end_of_stream:
                if (has_more_frames == 0)
                {
                    return 0;
                }

                has_more_frames = continuation_header->frame_number;
                return 1;
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
                return 1;
            }

            VOL_MOVIE_STATE->end_of_stream = TRUE;
            return 0;
        }
    }
    else
    {
        /* Validate and append an audio continuation sector. */
        continuation_header = &MOVIE_STATE->audio_data_base[VOL_MOVIE_STATE->audio_write_idx + MOVIE_STATE->chunk_sector_idx].header_block.header;
        while (CdGetSector(continuation_header, CD_HEADER_WORDS) == 0)
        {
        }

        if (((continuation_header->sector_type == SECTOR_TYPE_AUDIO) && (continuation_header->frame_number == MOVIE_STATE->frame_number)) &&
            (continuation_header->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            continuation_payload = MOVIE_STATE->audio_data_base[VOL_MOVIE_STATE->audio_write_idx + MOVIE_STATE->chunk_sector_idx].payload;
            while (CdGetSector(continuation_payload, CD_PAYLOAD_WORDS) == 0)
            {
            }

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (MOVIE_STATE->sectors_remaining == 0)
            {
                next_audio_write_idx = VOL_MOVIE_STATE->audio_write_idx + 1;
                VOL_MOVIE_STATE->audio_write_idx = next_audio_write_idx + MOVIE_STATE->chunk_sector_idx;

                MOVIE_STATE->last_audio_frame = VOL_MOVIE_STATE->frame_number;
                if (continuation_header->frame_number > MOVIE_STATE->total_frames)
                {
                    return 0;
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
                return 1;
            }

            VOL_MOVIE_STATE->end_of_stream = TRUE;
            return 0;
        }
    }

    return 1;
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
    if ((VOL_MOVIE_STATE->audio_write_idx <= MOVIE_STATE->audio_read_idx) && (MOVIE_STATE->audio_read_idx == MOVIE_STATE->audio_ring_size))
    {
        MOVIE_STATE->audio_read_idx = 0;

        if (MOVIE_STATE->audio_write_idx == 0 && (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
        {
            return 0;
        }
    }

    /* Skip entries already queued to the audio pipeline. */
    next_entry_idx = MOVIE_STATE->audio_read_idx + MOVIE_STATE->audio_buffered_count;

    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_entry_idx >= VOL_MOVIE_STATE->audio_ring_size))
    {
        next_entry_idx -= MOVIE_STATE->audio_ring_size;
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
    volatile MovieState* state = VOL_MOVIE_STATE;

    if (g_busy)
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

    write_index = VOL_MOVIE_STATE->video_write_idx;
    read_index = VOL_MOVIE_STATE->video_read_idx;

    /* Wrap after consuming the previous contiguous ring segment. */
    if ((read_index >= write_index) && (read_index == MOVIE_STATE->video_ring_size))
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
    if ((MOVIE_STATE->video_read_idx >= MOVIE_STATE->video_write_idx) && (next_read_index == MOVIE_STATE->video_ring_size))
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
    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_read_index == MOVIE_STATE->video_ring_size))
    {
        next_read_index = 0;
    }

    /* Commit the completed frame and next consumer position. */
    MOVIE_STATE->last_consumed_audio_frame = frame_header->frame_number;
    MOVIE_STATE->audio_read_idx = next_read_index;
}
