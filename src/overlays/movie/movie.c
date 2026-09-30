#include "movie_internal.h"
#include "field_scene.h"
#include "sdk/libetc.h"

/** @brief Fixed configuration used by @ref movie_play. */
#define MOVIE_DISPLAY_WIDTH 320
#define MOVIE_DISPLAY_HEIGHT 240
#define MOVIE_RGB24_VRAM_WIDTH ((MOVIE_DISPLAY_WIDTH * 3) / 2)
#define STANDARD_DECODE_RECT_WIDTH 24
#define ALTERNATE_DECODE_RECT_WIDTH 16
#define ALTERNATE_RECT_WRAP_THRESHOLD 768
#define ALTERNATE_RECT_WRAP_X 512
#define MOVIE_UPDATE_POLL_LIMIT 8192
#define CONTROLLER_VSYNC_INTERVAL_SINGLE 1
#define MOVIE_FRAME_VSYNC_INTERVAL 4
#define CD_ERROR_STATUS_RETRIES_EXHAUSTED 5

/** @brief Per-stream frame totals used as the playback stop condition. */
#define MOVIE_INTRO_TOTAL_FRAMES 2098
#define MOVIE_ATTRACT_1_TOTAL_FRAMES 2473
#define MOVIE_ATTRACT_2_PART_1_TOTAL_FRAMES 1318
#define MOVIE_ATTRACT_2_PART_2_TOTAL_FRAMES 5368
#define MOVIE_ATTRACT_2_PART_3_TOTAL_FRAMES 898

/** @brief Skip-cinematic gating used by movie_play. */
#define MOVIE_FIRST_UNSKIPPABLE_INDEX MOVIE_INDEX_ATTRACT_2_PART_1
#if defined(VERSION_JP)
#define MOVIE_INTRO_SKIP_MASK (PAD_BTN_R2 | PAD_BTN_R1 | PAD_BTN_RIGHT)
#else
#define MOVIE_INTRO_SKIP_MASK ((u16) ~(PAD_BTN_SQUARE | PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_TRIANGLE))
#endif
#define MOVIE_ATTRACT_1_SKIP_MASK (PAD_BTN_R2 | PAD_BTN_R1 | PAD_BTN_DOWN)
#define SCD_VALID_DEVICE_TYPE_COUNT 3 /**< digital, analog joystick, analog controller */

/** @brief Movie resource-table base, index representation, and initialization flag. */
#define MOVIE_RESOURCE_BASE 0x16A0
#define MOVIE_INIT_GPU_MODE_MASK 0x7F
#define MOVIE_INIT_USE_CD_AUDIO 0x80

/**
 * @brief Audio fade-out ramp during a skip-triggered exit.
 *
 * Armed by setting audio_fade_vol = AUDIO_FADE_INITIAL, stepped down by
 * AUDIO_FADE_STEP each outer-loop iteration, exits the loop when it reaches 0.
 */
#define AUDIO_FADE_DISARMED (-1)
#define AUDIO_FADE_INITIAL 112
#define AUDIO_FADE_STEP 16

/** @brief Movie initialization sentinels and audio parameters. */
#define MOVIE_FRAME_NONE ((u32) - 1)
#define AKAO_CD_VOLUME_MAX 127
#define MOVIE_AKAO_C8_INIT_VALUE 0x7FFF
#define MOVIE_NONSTREAMED_CD_MIX_VOLUME 160

/** @brief Ring capacities for the two movie buffer layouts. */
#define STANDARD_VIDEO_RING_SLOTS 50
#define ALTERNATE_VIDEO_RING_SLOTS 30
#define AUDIO_RING_SLOTS 16

/** @brief Fixed RAM layout used by the standard movie path. */
#define MOVIE_BUFFER_RAM_BASE SECONDARY_OVERLAY_AT(0x7000)
#define MOVIE_VLC_TABLE_BYTES 69632
#define STANDARD_VLC_INPUT_BYTES 81920
#define STANDARD_MDEC_OUTPUT_BYTES 11520

/** @brief Full-screen video, audio and decoder storage in the movie arena. */
typedef struct
{
    VideoSectorEntry video_table[STANDARD_VIDEO_RING_SLOTS];
    VideoVlcPayload video_data[STANDARD_VIDEO_RING_SLOTS];
    AudioSector audio_data[AUDIO_RING_SLOTS];
    u8 vlc_table[MOVIE_VLC_TABLE_BYTES];
    u8 vlc_input_buf[2][STANDARD_VLC_INPUT_BYTES];
    u_long mdec_output_buf[2][STANDARD_MDEC_OUTPUT_BYTES / sizeof(u_long)];
} StandardMovieBuffers;

#define STANDARD_MOVIE_BUFFERS ((StandardMovieBuffers*)MOVIE_BUFFER_RAM_BASE)

/** @brief Fixed RAM layout used by the alternate movie path. */
#define ALTERNATE_VLC_INPUT_BYTES 69632

/** @brief FIELD movie rings and VLC input buffers in the movie arena. */
typedef struct
{
    VideoSectorEntry video_table[ALTERNATE_VIDEO_RING_SLOTS];
    VideoVlcPayload video_data[ALTERNATE_VIDEO_RING_SLOTS];
    AudioSector audio_data[AUDIO_RING_SLOTS];
    u8 vlc_input_buf[2][ALTERNATE_VLC_INPUT_BYTES];
} AlternateMovieBuffers;

#define ALTERNATE_MOVIE_BUFFERS ((AlternateMovieBuffers*)MOVIE_BUFFER_RAM_BASE)

/** @brief VLC and MDEC storage reserved by the FIELD scene. */
#define ALTERNATE_MDEC_OUTPUT_BYTES 7680

/** @brief FIELD-owned decode table and two MDEC output slices. */
typedef struct
{
    u8 vlc_table[MOVIE_VLC_TABLE_BYTES];
    u_long mdec_output_buf[2][ALTERNATE_MDEC_OUTPUT_BYTES / sizeof(u_long)];
} AlternateMovieDecodeBuffers;

/** @brief A queued frame is either a video bitstream or an AKAO audio block. */
typedef union
{
    VideoVlcPayload* video;
    AudioSector* audio;
} MovieFrameData;

/**
 * @brief Play the selected MDEC cinematic.
 * @param movie_index Cinematic index (0..4).
 * @see https://decomp.me/scratch/gkEWm (100%)
 */
void movie_play(u16 movie_index)
{
    DISPENV display_envs[2];
    DISPENV* display_env;
    MovieState* state;
    s32 audio_fade_vol;
    s32 retry_exhausted_status;
    s32 error_status;
    s32 update_poll_budget;
#if !defined(VERSION_JP)
    u16 movie_index_low;
#endif
    u16 buttons;
    s32 frame_count;
#if !defined(VERSION_JP)
    u32 movie_index_value;
#endif
    s32 resource_index;
    s32 init_flags;

    /* Refresh controller and CD state before honoring an intro skip. */
    VSync(0);
    update_controllers();
    set_controller_vsync_interval(CONTROLLER_VSYNC_INTERVAL_SINGLE);
    VSync(0);
    update_controllers();
    cdrom_process_state();
    if (((movie_index == MOVIE_INDEX_INTRO) && ((SCD_REGS)->device_type < SCD_VALID_DEVICE_TYPE_COUNT)) &&
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
    switch (movie_index)
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
    resource_index = movie_index + MOVIE_RESOURCE_BASE;
    movie_init(resource_index, init_flags, frame_count, 0);
    VSync(0);
    update_controllers();
    audio_fade_vol = AUDIO_FADE_DISARMED;
    retry_exhausted_status = CD_ERROR_STATUS_RETRIES_EXHAUSTED;
    state = MOVIE_STATE;

    while (1)
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

            while (1)
            {
                movie_update();

                if (state->frame_ready != 0)
                {
                    break;
                }

                if (state->end_state == END_STATE_DONE)
                {
                    reset_controller_vsync_state();
                    cdrom_reset();
                    DrawSync(0);
                    VSync(0);
                    SetDispMask(0);
                    return;
                }

                update_poll_budget--;
                movie_service_video_ops();

                if (update_poll_budget == 0)
                {
                    break;
                }
            }

            if (update_poll_budget == 0)
            {
                cdrom_process_state();
            }
        }

        /* Present the completed buffer and process skip input. */
        state->frame_ready = 0;
        set_controller_vsync_interval(MOVIE_FRAME_VSYNC_INTERVAL);
#if !defined(VERSION_JP)
        movie_index_low = movie_index;
#endif
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

#if defined(VERSION_JP)
        if ((movie_index < MOVIE_FIRST_UNSKIPPABLE_INDEX) && ((SCD_REGS)->device_type < SCD_VALID_DEVICE_TYPE_COUNT))
#else
        movie_index_value = movie_index_low;
        if ((movie_index_value < MOVIE_FIRST_UNSKIPPABLE_INDEX) && ((SCD_REGS)->device_type < SCD_VALID_DEVICE_TYPE_COUNT))
#endif
        {
            buttons = (SCD_REGS)->pressed_buttons;
#if defined(VERSION_JP)
            if ((buttons & MOVIE_INTRO_SKIP_MASK) != 0)
#else
            if (movie_index_value != MOVIE_INDEX_INTRO ? (buttons & MOVIE_ATTRACT_1_SKIP_MASK) != 0 : (buttons & MOVIE_INTRO_SKIP_MASK) != 0)
#endif
            {
#if defined(VERSION_JP)
                if (state->use_cd_audio == 0)
#else
                if (g_movie_use_cd_audio == 0)
#endif
                {
                    break;
                }

                if (audio_fade_vol == AUDIO_FADE_DISARMED)
                {
                    audio_fade_vol = AUDIO_FADE_INITIAL;
                }
            }
        }

        /* A skip fades the streamed audio before leaving playback. */
        if ((g_movie_use_cd_audio != 0) && (audio_fade_vol != AUDIO_FADE_DISARMED))
        {
            akao_set_xa_volume(audio_fade_vol);

            if (audio_fade_vol == 0)
            {
                break;
            }

            audio_fade_vol -= AUDIO_FADE_STEP;
        }

        if (state->end_state == END_STATE_DONE)
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
    FieldScene* scene = g_field_scene.scene;
    MovieState* state;

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
        MOVIE_STATE->chunk_idx = 0;
    }
    else
    {
        /* Configure the alternate layout around the FIELD scene's VLC storage. */
        MOVIE_STATE->video_table_base = ALTERNATE_MOVIE_BUFFERS->video_table;
        MOVIE_STATE->audio_data_base = ALTERNATE_MOVIE_BUFFERS->audio_data;
        MOVIE_STATE->vlc_table = ((AlternateMovieDecodeBuffers*)scene->vlc_table)->vlc_table;
        MOVIE_STATE->vlc_input_buf[0] = ALTERNATE_MOVIE_BUFFERS->vlc_input_buf[0];
        MOVIE_STATE->vlc_input_buf[1] = ALTERNATE_MOVIE_BUFFERS->vlc_input_buf[1];
        MOVIE_STATE->mdec_output_buf[0] = ((AlternateMovieDecodeBuffers*)scene->vlc_table)->mdec_output_buf[0];
        MOVIE_STATE->mdec_output_buf[1] = ((AlternateMovieDecodeBuffers*)scene->vlc_table)->mdec_output_buf[1];
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
        MOVIE_STATE->video_data_base = (VideoVlcPayload*)(MOVIE_STATE->video_table_base + ALTERNATE_VIDEO_RING_SLOTS);
        MOVIE_STATE->chunk_idx = init_buffer_idx;
    }

    state = MOVIE_STATE;

    /* Reset stream counters and pipeline state. */
    state->resource_index = resource_index;
    state->current_frame = 0;
    state->total_frames = total_frames;
    state->input_buf_idx = 0;
    state->vlc_retry_count = 0;
    state->mdec_retry_pending = 0;
    state->video_service_busy = 0;
    state->draw_sync_target = 0;
    state->out_buf_idx = 0;
    state->pending_vram_upload = 0;
    state->pending_mdec_decode = 0;
    state->mdec_busy = MDEC_STATE_IDLE;
    state->frame_ready = 0;
    state->end_of_stream = 0;
    state->end_state = END_STATE_RUNNING;
    state->audio_stream_state = AUDIO_STREAM_STATE_IDLE;
    state->video_write_idx = 0;
    state->video_read_idx = 0;
    state->video_wrap_idx = 0;
    state->audio_write_idx = 0;
    state->audio_read_idx = 0;
    state->audio_wrap_idx = 0;
    state->audio_buffered_count = 0;
    state->frame_number = 0;
    state->continuation_type = 0;
    state->sectors_remaining = 0;
    state->last_video_frame = MOVIE_FRAME_NONE;
    state->last_consumed_video_frame = MOVIE_FRAME_NONE;
    state->last_audio_frame = MOVIE_FRAME_NONE;
    state->last_consumed_audio_frame = MOVIE_FRAME_NONE;

    /* Install movie callbacks and retain the previous handlers. */
    state->dec_dct_out_callback.address = DecDCToutCallback(&movie_mdec_out_callback);
    state->draw_sync_callback.address = DrawSyncCallback(&draw_sync_callback);

    /* Configure audio for streamed or non-streamed playback. */
    if (state->use_cd_audio != 0)
    {
        akao_start_xa_stream((s32)state->audio_data_base, state->audio_ring_capacity * sizeof(AudioSector));
        akao_set_xa_volume(AKAO_CD_VOLUME_MAX);
    }
    else
    {
        akao_set_cd_volume(MOVIE_AKAO_C8_INIT_VALUE);
        akao_set_cd_mix(MOVIE_NONSTREAMED_CD_MIX_VOLUME);
    }

    /* Queue the first streaming-sector read. */
    cdrom_wait_queue_empty();
    cdrom_queue_command(CdlReadS, (s16)resource_index, NULL, cd_sector_callback);

    state = MOVIE_STATE;

    /* Prepare display memory and VLC tables for the standard GPU path. */
    if (g_gpu_mode == MOVIE_GPU_MODE_STANDARD)
    {
        VSync(0);
        SetDispMask(0);
        ClearImage(&state->rects[0], 0, 0, 0);
        ClearImage(&state->rects[1], 0, 0, 0);
        DecDCTvlcBuild(state->vlc_table);
        DrawSync(0);
    }
}

/**
 * @brief Advance video decoding and the AKAO audio stream.
 * @see https://decomp.me/scratch/NpM84 (100%)
 */
void movie_update(void)
{
    s32 audio_ring_capacity;
    MovieFrameData frame;
    VideoSectorEntry* stream_header;
    s32 vlc_decode_complete = 0;
    MovieState* movie_state = MOVIE_STATE;

    /* Retry a deferred MDEC submission when the decoder is idle. */
    if (g_movie_mdec_retry_pending != 0)
    {
        if ((MOVIE_STATE->mdec_busy == MDEC_STATE_IDLE) && (movie_state->frame_ready == 0))
        {
            s32 pixel_count;

            MOVIE_STATE->mdec_busy = MDEC_STATE_ACTIVE;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD);
            pixel_count = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w * MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h;
            DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], pixel_count / 2);
            MOVIE_STATE->mdec_retry_pending = 0;
        }
    }

    /* Continue VLC decoding or begin the next buffered video frame. */
    if (g_movie_mdec_retry_pending == 0)
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
        else if (get_next_video_entry(&frame.video, &stream_header) != 0)
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
            if (DecDCTvlc2(frame.video->words, MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->vlc_table) == 0)
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
        /* Submit the decoded frame or defer it until the MDEC is idle. */
        advance_video_read();

        if ((MOVIE_STATE->mdec_busy == MDEC_STATE_IDLE) && (MOVIE_STATE->frame_ready == 0))
        {
            s32 pixel_count;

            MOVIE_STATE->mdec_busy = MDEC_STATE_ACTIVE;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->gpu_mode == MOVIE_GPU_MODE_STANDARD);
            pixel_count = MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].w * MOVIE_STATE->rects[MDEC_OUTPUT_RECT_INDEX].h;
            DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], pixel_count / 2);
        }
        else
        {
            g_movie_mdec_retry_pending = 1;
        }
    }

    /* Release audio sectors only after the SPU has consumed their samples. */
    movie_state = MOVIE_STATE;
    if (g_movie_use_cd_audio != 0)
    {
        if (get_next_audio_entry(&frame.audio) != 0)
        {
            stream_header = &frame.audio->header_block;
            movie_state->current_frame = stream_header->header.frame_number;

            if ((stream_header->header.frame_number > movie_state->total_frames) && (movie_state->end_state < END_STATE_DONE))
            {
                movie_state->end_state = END_STATE_DONE;
            }
            akao_xa_advance_frame();
        }
        movie_state = MOVIE_STATE;
        if (g_movie_audio_stream_state == AUDIO_STREAM_STATE_PRIMED)
        {
            audio_ring_capacity = movie_state->audio_ring_capacity;

            if (MOVIE_STATE->audio_buffered_count >= (audio_ring_capacity >> 1))
            {
                akao_resume_audio(AKAO_COMMAND_SELECTOR_9E);
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
