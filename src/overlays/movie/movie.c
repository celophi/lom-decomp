#include "movie_internal.h"

/**
 * @brief Play the selected MDEC cinematic.
 * @param movie_index Cinematic index (0..4); other values use the final attract segment.
 * @see https://decomp.me/scratch/gkEWm (100%)
 */
void movie_play(s32 movie_index)
{
    DISPENV display_envs[2];
    DISPENV* display_env;
    volatile MovieState* state;
    s32 audio_fade_vol;
    s32 retry_exhausted_status;
    s8 end_state_match;
    s32 error_status;
    s32 update_poll_budget;
    u16 movie_index_low;
    u16 buttons;
    s32 frame_count;
    u32 movie_index_value;
    s32 resource_index;
    s32 init_flags;

    /* Refresh controller and CD state before honoring an intro skip. */
    VSync(0);
    update_controllers();
    set_controller_vsync_interval(CONTROLLER_VSYNC_INTERVAL_SINGLE);
    VSync(0);
    update_controllers();
    cdrom_process_state();
    if ((((movie_index & MOVIE_INDEX_MASK) == MOVIE_INDEX_INTRO) && ((SCD_REGS)->device_type < SCD_VALID_DEVICE_TYPE_COUNT)) &&
        (((SCD_REGS)->held_buttons & MOVIE_INTRO_SKIP_MASK) != 0))
    {
        return;
    }

    /* Configure vertically stacked RGB24 display buffers. */
    reset_controller_vsync_state();
    DecDCTReset(0);
    SetDefDispEnv(&display_envs[0], 0, 0, MOVIE_DISPLAY_WIDTH, MOVIE_DISPLAY_HEIGHT);
    SetDefDispEnv(&display_envs[1], 0, MOVIE_DISPLAY_HEIGHT, MOVIE_DISPLAY_WIDTH, MOVIE_DISPLAY_HEIGHT);
    display_envs[1].isrgb24 = 1;
    display_envs[0].isrgb24 = 1;

    /* Select the stream length; movie resources are contiguous by index. */
    switch (movie_index & MOVIE_INDEX_MASK)
    {
    case MOVIE_INDEX_INTRO:
        frame_count = MOVIE_INTRO_TOTAL_FRAMES;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case MOVIE_INDEX_ATTRACT_1:
        frame_count = MOVIE_ATTRACT_1_TOTAL_FRAMES;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case MOVIE_INDEX_ATTRACT_2_PART_1:
        frame_count = MOVIE_ATTRACT_2_PART_1_TOTAL_FRAMES;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case MOVIE_INDEX_ATTRACT_2_PART_2:
        frame_count = MOVIE_ATTRACT_2_PART_2_TOTAL_FRAMES;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case MOVIE_INDEX_ATTRACT_2_PART_3:

    default:
        frame_count = MOVIE_ATTRACT_2_PART_3_TOTAL_FRAMES;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;
    }

    /* Stage the selected stream and initialize playback state. */
    resource_index = (movie_index & MOVIE_INDEX_MASK) + MOVIE_RESOURCE_BASE;
    movie_init(resource_index, init_flags, frame_count, 0);
    VSync(0);
    update_controllers();
    audio_fade_vol = AUDIO_FADE_DISARMED;
    retry_exhausted_status = CD_ERROR_STATUS_RETRIES_EXHAUSTED;
    state = VOL_MOVIE_STATE;
    end_state_match = END_STATE_DONE;

    while (TRUE)
    {
        /* Service transient CD errors before advancing playback. */
        error_status = cdrom_get_error_status();

        while ((error_status != 0) && (error_status != retry_exhausted_status))
        {
            set_controller_vsync_interval(CONTROLLER_VSYNC_INTERVAL_SINGLE);
            VSync(0);
            update_controllers();
            cdrom_process_state();
            error_status = cdrom_get_error_status();
        }

        /* Pump the decoder until a frame is ready or the stream ends. */
        while (state->frame_ready == 0)
        {
            update_poll_budget = MOVIE_UPDATE_POLL_LIMIT;

            while (TRUE)
            {
                movie_update();

                if (state->frame_ready != 0)
                {
                    break;
                }

                if (state->end_state == end_state_match)
                {
                    reset_controller_vsync_state();
                    cdrom_reset();
                    DrawSync(0);
                    VSync(0);
                    SetDispMask(0);
                    return;
                }

                movie_service_video_ops();

                if (--update_poll_budget == 0)
                {
                    break;
                }
            };

            if (update_poll_budget == 0)
            {
                cdrom_process_state();
            }
        }

        /* Present the completed buffer and process skip input. */
        state->frame_ready = 0;
        set_controller_vsync_interval(MOVIE_FRAME_VSYNC_INTERVAL);
        movie_index_low = movie_index & MOVIE_INDEX_MASK;
        VSync(0);
        display_env = &display_envs[0];
        if (state->chunk_idx == 0)
        {
            display_env = &display_envs[1];
        }
        PutDispEnv(display_env);
        SetDispMask(1);
        update_controllers();
        cdrom_process_state();

        movie_index_value = movie_index_low;
        if ((movie_index_value < MOVIE_FIRST_UNSKIPPABLE_INDEX) && ((SCD_REGS)->device_type < SCD_VALID_DEVICE_TYPE_COUNT))
        {
            buttons = (SCD_REGS)->pressed_buttons;
            if (((movie_index_value != MOVIE_INDEX_INTRO) ? ((buttons & MOVIE_ATTRACT_1_SKIP_MASK) != 0) : ((buttons & MOVIE_INTRO_SKIP_MASK) != 0)) != 0)
            {
                if (g_cd_audio_ready == 0)
                {
                    break;
                }

                if (audio_fade_vol == AUDIO_FADE_DISARMED)
                {
                    audio_fade_vol = AUDIO_FADE_INITIAL;
                }
            }
        }

        /* Fade XA audio after a skip request. */
        if ((g_cd_audio_ready != 0) && (audio_fade_vol != AUDIO_FADE_DISARMED))
        {
            akao_cmd_e4_set_cd_volume(audio_fade_vol);

            if (audio_fade_vol == 0)
            {
                break;
            }

            audio_fade_vol -= AUDIO_FADE_STEP;
        }

        if (state->end_state == end_state_match)
        {
            break;
        }
    }

    /* Stop playback and disable display output. */
    reset_controller_vsync_state();
    cdrom_reset();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}

/**
 * @brief Initialize movie buffers and streaming state.
 * @param resource_index CD resource index for the movie stream.
 * @param flags Lower seven bits select the GPU mode; bit 7 enables CD/XA audio.
 * @param total_frames Number of frames in the stream.
 * @param init_buffer_idx Initial buffer index (0 or 1) for the alternate layout.
 * @see https://decomp.me/scratch/g5PtA (100%)
 */
void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx)
{
    AllocInfo* alloc_info = g_allocInfo;
    MovieState* ms;

    /* Decode the GPU and audio modes from the initialization flags. */
    MOVIE_STATE->gpu_mode = flags & MOVIE_INIT_GPU_MODE_MASK;
    if (flags & MOVIE_INIT_USE_CD_AUDIO)
    {
        MOVIE_STATE->use_cd_audio = 1;
    }
    else
    {
        MOVIE_STATE->use_cd_audio = 0;
    }
    if (MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD)
    {
        /* Configure the fixed-buffer layout used by normal playback. */
        MOVIE_STATE->video_table_base = STANDARD_MOVIE_BUFFERS->video_table;
        MOVIE_STATE->audio_data_base = STANDARD_MOVIE_BUFFERS->audio_data;
        MOVIE_STATE->vlc_table = STANDARD_MOVIE_BUFFERS->vlc_table;
        MOVIE_STATE->vlc_input_buf[0] = STANDARD_MOVIE_BUFFERS->vlc_input_buf[0];
        MOVIE_STATE->vlc_input_buf[1] = STANDARD_MOVIE_BUFFERS->vlc_input_buf[1];
        MOVIE_STATE->mdec_output_buf[0] = STANDARD_MOVIE_BUFFERS->mdec_output_buf[0];
        MOVIE_STATE->mdec_output_buf[1] = STANDARD_MOVIE_BUFFERS->mdec_output_buf[1];
        MOVIE_STATE->rects[1].x = 0;
        MOVIE_STATE->rects[0].x = 0;
        MOVIE_STATE->rects[0].y = 0;
        MOVIE_STATE->rects[1].y = MOVIE_DISPLAY_HEIGHT;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h = MOVIE_DISPLAY_HEIGHT;
        MOVIE_STATE->rects[1].w = MOVIE_RGB24_VRAM_WIDTH;
        MOVIE_STATE->rects[0].w = MOVIE_RGB24_VRAM_WIDTH;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w = STANDARD_DECODE_RECT_WIDTH;
        MOVIE_STATE->video_ring_capacity = STANDARD_VIDEO_RING_SLOTS;
        MOVIE_STATE->rects[1].h = MOVIE_DISPLAY_HEIGHT;
        MOVIE_STATE->rects[0].h = MOVIE_DISPLAY_HEIGHT;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x = 0;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].y = 0;
        MOVIE_STATE->audio_ring_capacity = AUDIO_RING_SLOTS;
        MOVIE_STATE->video_data_base = STANDARD_MOVIE_BUFFERS->video_data;
        VOL_MOVIE_STATE->chunk_idx = 0;
    }
    else
    {
        /* Configure the alternate layout around allocator-owned VLC storage. */
        MOVIE_STATE->video_table_base = ALTERNATE_MOVIE_BUFFERS->video_table;
        MOVIE_STATE->audio_data_base = ALTERNATE_MOVIE_BUFFERS->audio_data;
        MOVIE_STATE->vlc_table = alloc_info->alloc_base->vlc_table;
        MOVIE_STATE->vlc_input_buf[0] = ALTERNATE_MOVIE_BUFFERS->vlc_input_buf[0];
        MOVIE_STATE->vlc_input_buf[1] = ALTERNATE_MOVIE_BUFFERS->vlc_input_buf[1];
        MOVIE_STATE->mdec_output_buf[0] = alloc_info->alloc_base->mdec_output_buf[0];
        MOVIE_STATE->mdec_output_buf[1] = alloc_info->alloc_base->mdec_output_buf[1];
        if (MOVIE_STATE->rects[0].x >= ALTERNATE_RECT_WRAP_THRESHOLD)
        {
            MOVIE_STATE->rects[1].x = ALTERNATE_RECT_WRAP_X;
            MOVIE_STATE->rects[1].y = 0;
        }
        else
        {
            MOVIE_STATE->rects[1].x = MOVIE_STATE->rects[0].x + MOVIE_STATE->rects[0].w;
            MOVIE_STATE->rects[1].y = MOVIE_STATE->rects[0].y;
        }
        MOVIE_STATE->rects[1].w = MOVIE_STATE->rects[0].w;
        MOVIE_STATE->rects[1].h = MOVIE_STATE->rects[0].h;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h = MOVIE_STATE->rects[0].h;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].x = MOVIE_STATE->rects[init_buffer_idx].x;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].y = MOVIE_STATE->rects[init_buffer_idx].y;
        MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w = ALTERNATE_DECODE_RECT_WIDTH;
        MOVIE_STATE->video_ring_capacity = ALTERNATE_VIDEO_RING_SLOTS;
        MOVIE_STATE->audio_ring_capacity = AUDIO_RING_SLOTS;
        MOVIE_STATE->video_data_base = VIDEO_PAYLOADS_AFTER_TABLE(MOVIE_STATE->video_table_base, ALTERNATE_VIDEO_RING_SLOTS);
        VOL_MOVIE_STATE->chunk_idx = init_buffer_idx;
    }

    ms = MOVIE_STATE;

    /* Reset stream counters and pipeline state. */
    ms->resource_index = resource_index;
    ms->current_frame = 0;
    ms->total_frames = total_frames;
    ms->input_buf_idx = 0;
    ms->vlc_retry_count = 0;
    ms->mdec_retry_pending = 0;
    ms->busy = 0;
    ms->draw_sync_target = 0;
    ms->out_buf_idx = 0;
    ms->pending_vram_upload = 0;
    ms->pending_mdec_decode = 0;
    ms->mdec_busy = MDEC_STATE_IDLE;
    ms->frame_ready = 0;
    ms->end_of_stream = 0;
    ms->end_state = 0;
    ms->audio_stream_state = AUDIO_STREAM_STATE_IDLE;
    ms->video_write_idx = 0;
    ms->video_read_idx = 0;
    ms->video_ring_size = 0;
    ms->audio_write_idx = 0;
    ms->audio_read_idx = 0;
    ms->audio_ring_size = 0;
    ms->audio_buffered_count = 0;
    ms->frame_number = 0;
    ms->continuation_type = 0;
    ms->sectors_remaining = 0;
    ms->last_video_frame = MOVIE_FRAME_NONE;
    ms->last_consumed_video_frame = MOVIE_FRAME_NONE;
    ms->last_audio_frame = MOVIE_FRAME_NONE;
    ms->last_consumed_audio_frame = MOVIE_FRAME_NONE;

    /* Install movie callbacks and retain the previous handlers. */
    ms->dec_dct_out_callback.address = DecDCToutCallback(&movie_mdec_out_callback);
    ms->draw_sync_callback.address = DrawSyncCallback(&draw_sync_callback);

    /* Configure audio for streamed or non-streamed playback. */
    if (ms->use_cd_audio != 0)
    {
        akao_cmd_e8_start_xa_stream(AKAO_STREAM_ADDRESS(ms->audio_data_base), ms->audio_ring_capacity * sizeof(AudioSector));
        akao_cmd_e4_set_cd_volume(AKAO_CD_VOLUME_MAX);
    }
    else
    {
        akao_cmd_c8(MOVIE_AKAO_C8_INIT_VALUE);
        akao_xa_setup_panning(MOVIE_NONSTREAMED_CD_MIX_VOLUME);
    }

    /* Queue the first streaming-sector read. */
    cdrom_wait_queue_empty();
    cdrom_queue_command(CdlReadS, (s16)resource_index, NULL, cd_sector_callback);

    ms = MOVIE_STATE;

    /* Prepare display memory and VLC tables for the standard GPU path. */
    if (g_gpu_mode == MOVIE_GPU_MODE_STANDARD)
    {
        VSync(0);
        SetDispMask(0);
        ClearImage(&ms->rects[0], 0, 0, 0);
        ClearImage(&ms->rects[1], 0, 0, 0);
        DecDCTvlcBuild(ms->vlc_table);
        DrawSync(0);
    }
}

/**
 * @brief Advance movie video decoding and XA audio playback.
 * @see https://decomp.me/scratch/NpM84 (100%)
 */
void movie_update(void)
{
    s32 audio_ring_capacity;

    MovieStreamEntryPointer stream_entry;
    VideoSectorEntry* stream_header;

    s32 vlc_decode_complete = 0;
    MovieState* movie_state = MOVIE_STATE;

    /* Retry a deferred MDEC submission when the decoder is idle. */
    if (g_mdecRetryPending != 0)
    {
        if ((MOVIE_STATE->mdec_busy == MDEC_STATE_IDLE) && (movie_state->frame_ready == 0))
        {
            MOVIE_STATE->mdec_busy = MDEC_STATE_ACTIVE;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD);
            {
                s32 pixel_count = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w * MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h;
                s32 word_count = pixel_count + SIGNED_HALF_ROUNDING(pixel_count);
                DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], word_count >> 1);
            }
            MOVIE_STATE->mdec_retry_pending = 0;
        }
    }

    /* Continue VLC decoding or begin the next buffered video frame. */
    if (g_mdecRetryPending == 0)
    {
        u8 retry_count = MOVIE_STATE->vlc_retry_count;
        if (retry_count != 0)
        {
            retry_count--;
            MOVIE_STATE->vlc_retry_count = retry_count;
            if (retry_count == 0)
            {
                DecDCTvlcSize2(0);
            }
            if (DecDCTvlc2(0, 0, MOVIE_STATE->vlc_table) == 0)
            {
                vlc_decode_complete = 1;
                MOVIE_STATE->vlc_retry_count = 0;
            }
        }
        else if (get_next_video_entry(&stream_entry.video_payload, &stream_header) != 0)
        {
            MOVIE_STATE->current_frame = stream_header->header.frame_number;

            if ((stream_header->header.frame_number >= MOVIE_STATE->total_frames) && (MOVIE_STATE->end_state == END_STATE_RUNNING))
            {
                MOVIE_STATE->end_state = END_STATE_NEAR_END;
            }

            MOVIE_STATE->input_buf_idx = 1 - MOVIE_STATE->input_buf_idx;

            if (MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD)
            {
                DecDCTvlcSize2(STANDARD_VLC_DECODE_SIZE);
                MOVIE_STATE->vlc_retry_count = STANDARD_VLC_RETRY_COUNT;
            }
            else
            {
                DecDCTvlcSize2(ALTERNATE_VLC_DECODE_SIZE);
                MOVIE_STATE->vlc_retry_count = ALTERNATE_VLC_RETRY_COUNT;
            }
            if (DecDCTvlc2(stream_entry.video_payload->words, MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->vlc_table) == 0)
            {
                vlc_decode_complete = 1;
                MOVIE_STATE->vlc_retry_count = 0;
            }
        }
        else if ((MOVIE_STATE->end_of_stream != 0) && (MOVIE_STATE->mdec_busy == MDEC_STATE_IDLE))
        {
            MOVIE_STATE->end_state = END_STATE_DONE;
        }
    }

    if (vlc_decode_complete != 0)
    {
        s32 output_available;
        s32 rounding_adjustment;

        /* Submit the decoded frame or defer it until the MDEC is idle. */
        advance_video_read();

        if ((MOVIE_STATE->mdec_busy == MDEC_STATE_IDLE) && (output_available = (MOVIE_STATE->frame_ready == 0)))
        {
            MOVIE_STATE->mdec_busy = MDEC_STATE_ACTIVE;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD);
            {
                s32 pixel_count = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w * MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h;
                rounding_adjustment = SIGNED_HALF_ROUNDING(pixel_count);
                DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], (pixel_count + rounding_adjustment) >> 1);
            }
        }
        else
        {
            g_mdecRetryPending = 1;
        }
    }

    /* Advance XA playback and retire consumed audio sectors. */
    movie_state = MOVIE_STATE;
    if (g_cd_audio_ready != 0)
    {
        if (get_next_audio_entry(&stream_entry.audio_sector) != 0)
        {
            stream_header = stream_entry.video_header;
            movie_state->current_frame = stream_header->header.frame_number;

            if ((stream_header->header.frame_number > movie_state->total_frames) && (movie_state->end_state < END_STATE_DONE))
            {
                movie_state->end_state = END_STATE_DONE;
            }
            akao_xa_advance_frame();
        }
        movie_state = MOVIE_STATE;
        if (g_audioStreamState == AUDIO_STREAM_STATE_PRIMED)
        {
            audio_ring_capacity = movie_state->audio_ring_capacity;

            if (MOVIE_STATE->audio_buffered_count >= (audio_ring_capacity >> 1))
            {
                akao_cmd_98_9a_9c_9e(AKAO_COMMAND_SELECTOR_9E);
                MOVIE_STATE->audio_stream_state = AUDIO_STREAM_STATE_IDLE;
            }
        }

        if ((MOVIE_STATE->audio_write_idx != MOVIE_STATE->audio_read_idx) || (MOVIE_STATE->last_audio_frame != MOVIE_STATE->last_consumed_audio_frame))
        {
            s32 position = akao_xa_get_position();
            if (((position != AKAO_XA_POSITION_UNAVAILABLE) && (MOVIE_STATE->audio_buffered_count != 0)) &&
                (MOVIE_STATE->audio_read_idx != (position * AUDIO_SECTORS_PER_XA_FRAME)))
            {
                advance_audio_read();
            }
        }
    }
}
