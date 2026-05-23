#include "movie.h"

static s32 get_next_audio_entry(AudioSector** out_entry);
static void draw_sync_callback(void);
static s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header);
static void advance_audio_read(void);
static void advance_video_read(void);

/**
 * @brief Play one of five MDEC cinematics, selected by index.
 *
 * Drives the full playback loop: configures the dual-buffered DISPENV pair,
 * resolves the per-movie frame count, calls @ref movie_init to stage the
 * stream, then services @ref movie_update / @ref movie_service_video_ops
 * until the stream end-state is reached. Includes a CD error-recovery
 * inner loop and an optional audio fade-out.
 *
 * @param movie_index Cinematic to play (0..4). Indices outside this range
 *                   fall through to the case-4 default (898 frames).
 *
 * @see https://decomp.me/scratch/gkEWm (100%)
 */
void movie_play(s32 movie_index)
{
    DISPENV env[2];
    DISPENV* p_disp_env;
    volatile MovieState* state;
    s32 audio_fade_vol;
    s32 retry_limit;
    char end_state_match;
    s32 error_status;
    s32 timeout;
    unsigned short movie_idx_u16;
    u16 buttons;
    s32 frame_count;
    u32 idx;
    s32 resource_idx;
    s32 init_flags;

    VSync(0);
    func_800157DC();
    func_800157B0(1);
    VSync(0);
    func_800157DC();
    cdrom_process_state();
    /* Pre-playback skip gate: if user is already holding a skip button on
     * movie 0 when we get here, bail before staging the stream. */
    if ((((movie_index & 0xFFFF) == 0) && ((SCD_REGS)->deviceState < SCD_DEVICE_STATE_OK)) &&
        (((SCD_REGS)->buttonData & MOVIE0_SKIP_MASK) != 0))
    {
        return;
    }
    func_800158E0();
    DecDCTReset(0);
    timeout = 0xF0;
    SetDefDispEnv(&env[0], 0, 0, 320, timeout);
    SetDefDispEnv(&env[1], 0, timeout, 320, timeout);
    (env[1].isrgb24 = 1);
    env[0].isrgb24 = 1;

    /*
     * Five MDEC cinematics; movie_index selects one (0..4). The frame count
     * matches each movie's BS stream length and is used by movie_init to set
     * the movie_init flags stop condition. The CD resource index is the per-movie
     * BS file at base 0x16A0 (so resources 0x16A0..0x16A4).
     *
     *   index | resource | frames | known role
     *   ------+----------+--------+----------------------
     *     0   | 0x16A0   |  2098  | (TODO: identify)
     *     1   | 0x16A1   |  2473  | (TODO: identify)
     *     2   | 0x16A2   |  1318  | (TODO: identify)
     *     3   | 0x16A3   |  5368  | (TODO: identify; longest — likely ending)
     *     4   | 0x16A4   |   898  | (TODO: identify; shortest — also default)
     *
     * To label these semantically, grep callers of `movie_play` to see which
     * index is invoked from where (intro screen, ending, etc.).
     */

    switch ((u16)(movie_index & 0xFFFF))
    {
    case 0:
        frame_count = 2098;
        init_flags = 0x80;
        break;

    case 1:
        frame_count = 2473;
        init_flags = 0x80;
        break;

    case 2:
        frame_count = 1318;
        init_flags = 0x80;
        break;

    case 3:
        frame_count = 5368;
        init_flags = 0x80;
        break;

    case 4:

    default:
        frame_count = 898;
        init_flags = 0x80;
        break;
    }

    resource_idx = (movie_index & 0xFFFF) + 0x16A0;
    movie_init(resource_idx, init_flags, frame_count, 0);
    VSync(0);
    func_800157DC();
    audio_fade_vol = AUDIO_FADE_DISARMED;
    retry_limit = 5;
    state = (MovieState*)0x801ED500;
    end_state_match = END_STATE_DONE;

    while (TRUE)
    {
        error_status = cdrom_get_error_status();

        while ((error_status != 0) && (error_status != retry_limit))
        {
            func_800157B0(1);
            VSync(0);
            func_800157DC();
            cdrom_process_state();
            error_status = cdrom_get_error_status();
        }

        while (state->frame_ready == 0)
        {
            timeout = 0x2000;

            while (TRUE)
            {
                movie_update();

                if (state->frame_ready != 0)
                {
                    break;
                }

                if (state->end_state == end_state_match)
                {
                    func_800158E0();
                    cdrom_reset();
                    DrawSync(0);
                    VSync(0);
                    SetDispMask(0);
                    return;
                }

                movie_service_video_ops();

                if (--timeout == 0)
                {
                    break;
                }
            };

            if (timeout == 0)
            {
                cdrom_process_state();
            }
        }

        state->frame_ready = 0;
        func_800157B0(4);
        movie_idx_u16 = (u16)(movie_index & 0xFFFF);
        VSync(0);
        p_disp_env = &env[0];
        if (state->chunk_idx == 0)
        {
            p_disp_env = &env[1];
        }
        PutDispEnv(p_disp_env);
        SetDispMask(1);
        func_800157DC();
        cdrom_process_state();

        idx = movie_idx_u16;
        if ((idx < MOVIE_SKIPPABLE_MAX) && ((SCD_REGS)->deviceState < SCD_DEVICE_STATE_OK))
        {
            buttons = (SCD_REGS)->unk4;
            if (((idx != 0) ? ((buttons & MOVIE1_SKIP_MASK) != 0) : ((buttons & MOVIE0_SKIP_MASK) != 0)) != 0)
            {
                if (g_cdAudioReady == 0)
                {
                    break;
                }

                if (audio_fade_vol == AUDIO_FADE_DISARMED)
                {
                    audio_fade_vol = AUDIO_FADE_INITIAL;
                }
            }
        }

        if ((g_cdAudioReady != 0) && (audio_fade_vol != AUDIO_FADE_DISARMED))
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

    func_800158E0();
    cdrom_reset();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}

/**
 * @brief Stage a movie stream for playback into the global @ref MovieState.
 *
 * Allocates VRAM rectangles, sets up the VLC table / MDEC output buffers /
 * audio data buffer, registers the DecDCT-out and DrawSync callbacks, then
 * issues the initial CD read. Two layout paths exist depending on
 * @ref g_gpuMode (standard vs BreakDraw/dynamic-alloc_base).
 *
 * @param resource_index CD resource id of the BS stream (e.g. 0x16A0..0x16A4).
 * @param flags         Bit 0..6 -> MovieState.gpu_mode. Bit 7 -> MovieState.interlace_mode
 *                      (despite the name, this selects the *audio source*, not
 *                      GPU interlacing - see @note below).
 * @param total_frames   Frame-count stop condition; set into MovieState.total_frames.
 * @param init_buffer_idx Initial active chunk index (0 or 1). Path B uses it to
 *                      seed rects[2] from rects[init_buffer_idx].
 *
 * @note `MovieState.gpu_mode` and the `g_gpuMode` global are *the same byte* at
 *       0x801ED590 (offset 0x90 in MovieState). The write at the top of this
 *       function and the `if (g_gpuMode == 0)` branch read the same storage.
 *
 * @note In shipped play, `movie_play` always passes flags = 0x80, so
 *       gpu_mode = 0 (path A taken every time) and interlace_mode = 1 (XA audio).
 *       Path B is dead in production; preserved for matching, which is also why
 *       its read of an uninitialised rects[0].x (`>= 0x300` guard) is inert.
 *
 * @note `interlace_mode` controls audio source, not display interlacing:
 *         1 -> akao_cmd_e8_start_xa_stream + cd_volume   (real CD-XA streaming)
 *         0 -> akao_cmd_c8(0x7FFF) + xa_setup_panning    (SPU/synth path)
 *
 * @note Path-A memory map (RAM addresses are literals in this function):
 *           0x80147000  video_table_base   (50 x 32   = 0x640)
 *           0x80147640  video_data_base    (50 x 2016)
 *           0x80160000  audio_data_base    (16 x 2048 = 0x8000)
 *           0x80168000  vlc_table          (0x11000)
 *           0x80179000  vlc_input_buf[0]   (0x14000)
 *           0x8018D000  vlc_input_buf[1]   (0x14000)
 *           0x801A1000  mdec_output_buf[0] (0x2D00 = 24 x 240 x 2)
 *           0x801A3D00  mdec_output_buf[1] (0x2D00)
 *       Sizes match rects[2] = 24-wide x 240-tall macroblock decode column.
 *
 * @note Path-B memory map (relative to AllocInfo::alloc_base):
 *           0x80147000             video_table_base   (30 x 32 = 0x3C0)
 *           video_table_base+0x3C0 video_data_base
 *           0x80156000             audio_data_base
 *           0x8015E000             vlc_input_buf[0]   (0x11000)
 *           0x8016F000             vlc_input_buf[1]
 *           alloc_base             vlc_table
 *           alloc_base+0x11000     mdec_output_buf[0] (0x1E00 = 16 x 240 x 2)
 *           alloc_base+0x12E00     mdec_output_buf[1]
 *       rects[2] is 16 wide here.
 *
 * @see https://decomp.me/scratch/g5PtA (91.61%)
 * @see https://decomp.me/scratch/ICOiP (incorrect but better match)
 */
void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx)
{
    MovieState* movie_state;
    AllocInfo* alloc_info = g_allocInfo;
    short value;
    short value2;
    void* addr;
    unsigned short init_rect_y;

    MOVIE_STATE->gpu_mode = (s8)(flags & 0x7F);
    if (flags & 0x80)
    {
        MOVIE_STATE->interlace_mode = 1;
    }
    else
    {
        MOVIE_STATE->interlace_mode = 0;
    }

    if (MOVIE_STATE->gpu_mode == 0)
    {

        MOVIE_STATE->video_table_base = (VideoSectorEntry*)0x80147000;
        MOVIE_STATE->audio_data_base = (AudioSector*)0x80160000;
        MOVIE_STATE->vlc_table = (void*)0x80168000;
        MOVIE_STATE->vlc_input_buf[0] = (void*)0x80179000;
        MOVIE_STATE->vlc_input_buf[1] = (void*)0x8018D000;
        MOVIE_STATE->mdec_output_buf[0] = (u_long*)0x801A1000;
        MOVIE_STATE->mdec_output_buf[1] = (u_long*)0x801A3D00;

        MOVIE_STATE->rects[1].y = 0xF0;
        MOVIE_STATE->rects[2].h = 0xF0;
        MOVIE_STATE->rects[1].h = 0xF0;

        MOVIE_STATE->video_ring_capacity = 0x32;

        MOVIE_STATE->rects[0].h = 0xF0;

        MOVIE_STATE->rects[1].x = 0;
        MOVIE_STATE->rects[0].x = 0;
        MOVIE_STATE->rects[0].w = 0x1E0;
        MOVIE_STATE->rects[1].w = 0x1E0;
        MOVIE_STATE->rects[0].w = 0x1E0;
        MOVIE_STATE->rects[2].w = 0x18;
        MOVIE_STATE->rects[0].y = 0;

        MOVIE_STATE->rects[2].x = 0;
        MOVIE_STATE->rects[2].y = 0;

        MOVIE_STATE->audio_ring_capacity = 0x10;
        MOVIE_STATE->video_data_base = (void*)0x80147640;
        MOVIE_STATE->chunk_idx = 0;
    }
    else
    {
        MOVIE_STATE->video_table_base = (VideoSectorEntry*)0x80147000;
        MOVIE_STATE->audio_data_base = (AudioSector*)0x80156000;
        MOVIE_STATE->vlc_input_buf[0] = (void*)0x8015E000;
        MOVIE_STATE->vlc_input_buf[1] = (void*)0x8016F000;
        MOVIE_STATE->vlc_table = (void*)alloc_info->alloc_base;
        MOVIE_STATE->mdec_output_buf[0] = (u_long*)(alloc_info->alloc_base + 0x11000);
        MOVIE_STATE->mdec_output_buf[1] = (u_long*)(alloc_info->alloc_base + 0x12E00);

        if (((s16)MOVIE_STATE->rects[0].x) >= 0x300)
        {
            MOVIE_STATE->rects[1].x = 0x200;
            MOVIE_STATE->rects[1].y = 0;
        }
        else
        {
            MOVIE_STATE->rects[1].x = (u16)(MOVIE_STATE->rects[0].x + MOVIE_STATE->rects[0].w);
            MOVIE_STATE->rects[1].y = MOVIE_STATE->rects[0].y;
        }

        MOVIE_STATE->rects[1].w = MOVIE_STATE->rects[0].w;
        MOVIE_STATE->rects[1].h = MOVIE_STATE->rects[0].h;
        MOVIE_STATE->rects[2].h = MOVIE_STATE->rects[0].h;
        MOVIE_STATE->rects[2].x = MOVIE_STATE->rects[init_buffer_idx].x;
        init_rect_y = (unsigned short)MOVIE_STATE->rects[init_buffer_idx].y;
        MOVIE_STATE->rects[2].w = 0x10;
        MOVIE_STATE->video_ring_capacity = 0x1E;

        MOVIE_STATE->audio_ring_capacity = 0x10;

        MOVIE_STATE->video_data_base = (VideoVlcPayload*)((u32)MOVIE_STATE->video_table_base + 0x3C0);
        MOVIE_STATE->chunk_idx = (s8)init_buffer_idx;
        MOVIE_STATE->rects[2].y = init_rect_y;
    }

    MOVIE_STATE->resource_index = resource_index;
    MOVIE_STATE->current_frame = 0;
    MOVIE_STATE->total_frames = total_frames;
    MOVIE_STATE->input_buf_idx = 0;
    MOVIE_STATE->vlc_retry_count = 0;
    MOVIE_STATE->mdec_retry_pending = 0;
    MOVIE_STATE->busy = 0;
    MOVIE_STATE->draw_sync_target = 0;
    MOVIE_STATE->out_buf_idx = 0;
    MOVIE_STATE->pending_vram_upload = 0;
    MOVIE_STATE->pending_mdec_decode = 0;
    MOVIE_STATE->mdec_busy = 0;
    MOVIE_STATE->frame_ready = 0;
    MOVIE_STATE->end_of_stream = 0;
    MOVIE_STATE->end_state = END_STATE_RUNNING;
    MOVIE_STATE->unk92 = 0;

    MOVIE_STATE->video_write_idx = 0;
    MOVIE_STATE->video_read_idx = 0;
    MOVIE_STATE->video_ring_size = 0;
    MOVIE_STATE->audio_write_idx = 0;
    MOVIE_STATE->audio_read_idx = 0;
    MOVIE_STATE->audio_ring_size = 0;
    MOVIE_STATE->audio_buffered_count = 0;
    MOVIE_STATE->frame_number = 0;
    MOVIE_STATE->continuation_type = 0;

    MOVIE_STATE->sectors_remaining = 0;
    MOVIE_STATE->last_video_frame = (u32)(-1);
    MOVIE_STATE->last_consumed_video_frame = (u32)(-1);
    MOVIE_STATE->last_audio_frame = (u32)(-1);
    MOVIE_STATE->last_consumed_audio_frame = (u32)(-1);

    /* Psy-Q's DecDCToutCallback takes a single function pointer; the trailing
     * p1/p2/p3 are codegen scratch — they pin specific values into $a1/$a2/$a3
     * at the call site to reproduce the original register state, and are
     * ignored by the callee. Path A: (vlc_input_buf[1], vlc_input_buf[0], vlc_table).
     * Path B: ((u16)rects[init_buffer_idx].y, 0x11000, init_buffer_idx). */
    MOVIE_STATE->dec_dct_out_callback = (u32)DecDCToutCallback(&movie_mdec_out_callback);
    MOVIE_STATE->draw_sync_callback = DrawSyncCallback(&draw_sync_callback);

    if (MOVIE_STATE->interlace_mode != 0)
    {
        akao_cmd_e8_start_xa_stream((u32)MOVIE_STATE->audio_data_base, (u32)(MOVIE_STATE->audio_ring_capacity << 0xB));
        akao_cmd_e4_set_cd_volume(0x7F);
    }
    else
    {
        akao_cmd_c8(0x7FFF);
        akao_xa_setup_panning(0xA0);
    }

    cdrom_wait_queue_empty();
    /* Load-bearing volatile re-read: pins audio_data_base into a register before
     * the cdrom_queue_command call below so the original codegen is preserved. */
    if (!MOVIE_STATE->audio_data_base)
    {
    }
    cdrom_queue_command(CdlReadS, (s16)resource_index, NULL, &movie_cd_sector_callback);

    if (g_gpuMode == 0)
    {
        VSync(0);
        SetDispMask(0);
        ClearImage(&MOVIE_STATE->rects[0], 0, 0, 0);
        ClearImage(&MOVIE_STATE->rects[1], 0, 0, 0);
        DecDCTvlcBuild(MOVIE_STATE->vlc_table);
        DrawSync(0);
    }
}

/**
 * @brief Per-tick movie pump: feeds the MDEC and advances the audio queue.
 *
 * Three responsibilities:
 *   1. If a previous MDEC submit was deferred, retry it now that the MDEC is idle.
 *   2. Pull the next BS frame from the video ring, decode VLC, and submit to MDEC.
 *   3. Drain the audio ring into the AKAO XA stream and update playback position.
 *
 * @see https://decomp.me/scratch/NpM84 (100%)
 */
void movie_update(void)
{
    s32 audio_capacity; /* audio_ring_capacity reload */

    VideoVlcPayload* vlc_payload;   /* raw bitstream for the next video frame */
    VideoSectorEntry* entry_header; /* 32-byte sector header for the same frame */
    AudioSector* audio_entry;       /* next audio ring entry (header + payload) */

    s32 tmp = 0; /* vlc-complete flag, then audio_buffered_count compare */
    MovieState* combined = MOVIE_STATE;

    if (g_mdecRetryPending != 0)
    {
        if ((MOVIE_STATE->mdec_busy == 0) && (combined->frame_ready == 0))
        {
            MOVIE_STATE->mdec_busy = 1;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx],
                     (MOVIE_STATE->gpu_mode & 0xFFFFu) == 0);
            {
                s32 temp =
                    ((s16)MOVIE_STATE->rects[2].w) * ((s16)MOVIE_STATE->rects[2].h);
                s32 word_count = temp + (((unsigned)temp) >> 31);
                DecDCTout((MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx]), word_count >> 1);
            }
            MOVIE_STATE->mdec_retry_pending = 0;
        }
    }

    if ((g_mdecRetryPending == 0) & 0xFFFFu)
    {
        u8 retry_count = MOVIE_STATE->vlc_retry_count;
        if (retry_count != 0)
        {
            retry_count--;
            MOVIE_STATE->vlc_retry_count = retry_count;
            if ((retry_count & 0xFF) == 0)
            {
                DecDCTvlcSize2(0);
            }
            if (DecDCTvlc2(0, 0, MOVIE_STATE->vlc_table) == 0)
            {
                tmp = 1;
                MOVIE_STATE->vlc_retry_count = 0;
            }
        }
        else if (get_next_video_entry(&vlc_payload, &entry_header) != 0)
        {
            MOVIE_STATE->current_frame = entry_header->header.frame_number;

            if ((entry_header->header.frame_number >= MOVIE_STATE->total_frames) &&
                (MOVIE_STATE->end_state == END_STATE_RUNNING))
            {
                MOVIE_STATE->end_state = END_STATE_NEAR_END;
            }

            MOVIE_STATE->input_buf_idx = 1 - MOVIE_STATE->input_buf_idx;

            if (MOVIE_STATE->gpu_mode == 0)
            {
                DecDCTvlcSize2(0x1000);
                MOVIE_STATE->vlc_retry_count = 3;
            }
            else
            {
                DecDCTvlcSize2(0x16AA);
                MOVIE_STATE->vlc_retry_count = 1;
            }
            if (DecDCTvlc2((u_long*)vlc_payload, MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx],
                           MOVIE_STATE->vlc_table) == 0)
            {
                tmp = 1;
                MOVIE_STATE->vlc_retry_count = 0;
            }
        }
        else if ((MOVIE_STATE->end_of_stream != 0) && (MOVIE_STATE->mdec_busy == 0))
        {
            MOVIE_STATE->end_state = END_STATE_DONE;
        }
    }

    if (tmp != 0)
    {
        s32 field_9d_zero_flag;
        advance_video_read();

        if ((MOVIE_STATE->mdec_busy == 0) && (field_9d_zero_flag = MOVIE_STATE->frame_ready == 0))
        {
            MOVIE_STATE->mdec_busy = 1;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->gpu_mode == 0);
            {
                s32 temp =
                    ((s16)MOVIE_STATE->rects[2].w) * ((s16)MOVIE_STATE->rects[2].h);
                field_9d_zero_flag = ((unsigned)temp) >> 31;
                DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], (temp + field_9d_zero_flag) >> 1);
            }
        }
        else
        {
            g_mdecRetryPending = 1;
        }
    }

    combined = MOVIE_STATE;
    if (g_cdAudioReady != 0)
    {
        if (get_next_audio_entry((void*)&vlc_payload) != 0)
        {
            entry_header = (VideoSectorEntry*)vlc_payload;
            MOVIE_STATE->current_frame = ((AudioSector*)vlc_payload)->header.frame_number;

            if ((((AudioSector*)vlc_payload)->header.frame_number > MOVIE_STATE->total_frames) &&
                (MOVIE_STATE->end_state < END_STATE_DONE))
            {
                MOVIE_STATE->end_state = END_STATE_DONE;
            }
            akao_xa_advance_frame();
        }
        combined = MOVIE_STATE;
        if (g_audioStreamState == 2)
        {

            audio_capacity = MOVIE_STATE->audio_ring_capacity;

            if ((s32)MOVIE_STATE->audio_buffered_count >= ((s32)(audio_capacity >> 1)))
            {
                akao_cmd_98_9a_9c_9e(3);
                MOVIE_STATE->unk92 = 0;
            }
        }

        if ((MOVIE_STATE->audio_write_idx != MOVIE_STATE->audio_read_idx) ||
            (MOVIE_STATE->last_audio_frame != MOVIE_STATE->last_consumed_audio_frame))
        {
            s32 position = akao_xa_get_position();
            if (((position != -1) && (MOVIE_STATE->audio_buffered_count != 0)) &&
                (MOVIE_STATE->audio_read_idx != (position * 2)))
            {
                advance_audio_read();
            }
        }
    }
}

/**
 * @brief MDEC-output completion callback.
 *
 * Invoked when the MDEC finishes writing a decoded macroblock buffer.
 * Either uploads the result to VRAM immediately (LoadImage / LoadImage2)
 * or defers the upload by setting @ref MovieState::pending_vram_upload.
 *
 * @see https://decomp.me/scratch/HVkZ6 (100%)
 */
void movie_mdec_out_callback(void)
{
    volatile MovieState* base = (volatile MovieState*)0x801ED500;
    s32 temp;
    MovieState* bp_high;
    int zero_literal;

    if (g_gpuMode == 0)
    {
        if (((u8)g_cdStatusByte3) == 1)
        {
            cdrom_verify_recovery();
        }
        temp = DrawSync(1);
        if (temp < 2)
        {
            LoadImage(&MOVIE_STATE->rects[2], base->mdec_output_buf[base->out_buf_idx]);
            base->draw_sync_target = (s8)(temp + 1);
        }
        else
        {
            base->pending_vram_upload = 1U;
        }
        bp_high = (MovieState*)0x801e0000;
    }
    else
    {
        temp = (s32)BreakDraw();
        zero_literal = 0;
        if (temp != (-1))
        {
            LoadImage2(&MOVIE_STATE->rects[2], base->mdec_output_buf[base->out_buf_idx]);
            if (temp != zero_literal)
            {
                DrawOTag((u_long*)temp);
            }
            bp_high = (MovieState*)0x801e0000;
        }
        else
        {
            LoadImage(&MOVIE_STATE->rects[2], base->mdec_output_buf[base->out_buf_idx]);
        }
    }

    if (MOVIE_STATE->pending_vram_upload == zero_literal)
    {
        movie_schedule_next_decode();
        return;
    }
    MOVIE_STATE->mdec_busy = 1;
}

/**
 * @brief Advance the decode position and schedule the next MDEC decode.
 *
 * Steps the rect[2] frame position by its width. If still inside the current
 * chunk, kicks off DecDCTout immediately (or sets @ref MovieState::pending_mdec_decode
 * if the GPU is busy). Otherwise toggles to the other chunk, resets the frame
 * position, and signals @ref MovieState::frame_ready.
 *
 * @see https://decomp.me/scratch/E7XCZ (100%)
 */
void movie_schedule_next_decode(void)
{
    unsigned short next_out_buf_idx;
    u16 cur_frame_pos;
    u16 frame_step;
    u16 new_frame_pos;
    u16* new_var;
    s32 new_frame_pos_signed;
    s32 chunk_end;
    u32 decode_size;
    int decode_word_count;

    next_out_buf_idx = 1 - MOVIE_STATE->out_buf_idx;

    cur_frame_pos = MOVIE_STATE->rects[2].x;
    frame_step = MOVIE_STATE->rects[2].w;

    new_frame_pos = cur_frame_pos + frame_step;

    MOVIE_STATE->rects[2].x = new_frame_pos;

    new_frame_pos_signed = (s16)new_frame_pos;

    VOL_MOVIE_STATE->out_buf_idx = next_out_buf_idx;

    chunk_end =
        MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].x + MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].w;

    if (new_frame_pos_signed < chunk_end)
    {
        if (MOVIE_STATE->draw_sync_target < 2U)
        {
            decode_size = ((s16)frame_step) * MOVIE_STATE->rects[2].h;
            decode_word_count = ((int)(decode_size + (decode_size >> 31))) >> 1;
            DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], decode_word_count);
            VOL_MOVIE_STATE->mdec_busy = 2;
        }
        else
        {
            MOVIE_STATE->mdec_busy = 1;
            VOL_MOVIE_STATE->pending_mdec_decode = 1;
        }
    }
    else
    {
        /* advance to the next chunk and reset the frame position */
        MOVIE_STATE->chunk_idx = 1 - MOVIE_STATE->chunk_idx;
        MOVIE_STATE->rects[2].x = MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].x;
        MOVIE_STATE->rects[2].y = MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].y;
        VOL_MOVIE_STATE->frame_ready = 1;
        VOL_MOVIE_STATE->mdec_busy = 0;
        if (VOL_MOVIE_STATE->end_state == END_STATE_NEAR_END)
        {
            MOVIE_STATE->end_state = END_STATE_DONE;
        }
    }
}

/**
 * @brief Service pending video output operations.
 *
 * Two flags gate the two halves:
 *   - pending_vram_upload: a decoded frame is ready; DMA it into VRAM
 *     (LoadImage) and kick off the MDEC decode of the next frame.
 *   - pending_mdec_decode: new BS bitstream data is staged; feed it to the
 *     MDEC (DecDCTout).
 *
 * gpu_mode selects the transfer path:
 *   - 0: wait for DrawSync then use LoadImage (standard DMA).
 *   - non-zero: interrupt the current draw via BreakDraw then use LoadImage2.
 *
 * @note The s8 status-byte fields (busy/draw_sync_target) emit `lb` where the
 *       original asm used `lbu`. Currently 93.87% non-matching; revisit the
 *       field types (s8 → u8) if the percentage regresses further.
 *
 * @see https://decomp.me/scratch/JTTFr (100%)
 */
void movie_service_video_ops(void)
{
    volatile MovieState* G = (volatile MovieState*)0x801ED500;
    int word_count;
    u_long* break_draw_result;
    if (!G->pending_vram_upload)
    {
        if (!G->pending_mdec_decode)
        {
            return;
        }
    }
    if (VOL_MOVIE_STATE->gpu_mode == 0)
    {
        if (DrawSync(1) >= 2)
        {
            return;
        }
        if (VOL_MOVIE_STATE->pending_vram_upload)
        {
            MOVIE_STATE->busy = 1;
            if (VOL_MOVIE_STATE->pending_vram_upload)
            {

                LoadImage(&MOVIE_STATE->rects[2], G->mdec_output_buf[VOL_MOVIE_STATE->out_buf_idx]);
                VOL_MOVIE_STATE->draw_sync_target = DrawSync(1) + 1;
                VOL_MOVIE_STATE->pending_vram_upload = 0;
                movie_schedule_next_decode();
            }
            VOL_MOVIE_STATE->busy = 0;
        }
        G = VOL_MOVIE_STATE;
        if (G->pending_mdec_decode)
        {
            MOVIE_STATE->busy = 1;
            if (G->pending_mdec_decode)
            {
                s32 temp;

                /* word count = (width * height) / 2, rounded toward zero for signed values */
                temp = ((s32)G->rects[2].w) * ((s32)G->rects[2].h);
                word_count = temp + (((u32)temp) >> 31);
                DecDCTout(G->mdec_output_buf[G->out_buf_idx], word_count >> 1);
                G->pending_mdec_decode = 0;
            }
            MOVIE_STATE->busy = 0;
        }
    }
    else
    {
        /* BreakDraw path: interrupt the current draw primitive list to upload immediately */
        if (G->pending_vram_upload)
        {
            MOVIE_STATE->busy = 1;
            if (G->pending_vram_upload)
            {
                s32 bd;

                break_draw_result = BreakDraw();
                bd = (s32)break_draw_result;
                if (bd != (-1))
                {
                    LoadImage2(&MOVIE_STATE->rects[2], G->mdec_output_buf[G->out_buf_idx]);
                    if (bd != 0)
                    {
                        /* Resume the interrupted OTag list */
                        DrawOTag((u_long*)bd);
                    }
                    movie_schedule_next_decode();
                    G->pending_vram_upload = 0;
                }
            }
            g_busy = 0;
        }
    }
}

/**
 * @brief CD sector-arrival callback; invoked from the CD-ROM ISR per sector.
 *
 * Reads the 8-word (32-byte) sector header, classifies the sector as video
 * (type 0x8001) or audio, checks whether the corresponding ring buffer has
 * room, then copies the 504-word (2016-byte) payload into the buffer and
 * advances the write index. Multi-sector frames are handled via
 * @ref g_sectorsRemaining: when 0 the sector is the first (header) sector;
 * when non-zero we are reading continuation sectors for the same frame and
 * skip the ring-capacity check.
 *
 * @return 1 to keep streaming, 0 when the stream has ended or should pause.
 *
 * @see https://decomp.me/scratch/5flHR (99.38%)
 */
s32 movie_cd_sector_callback(void)
{
    u32 hdr[8];
    int new_var2;
    s32 temp_a1_2;
    s32 flag;
    s32 has_more_frames;

    u32* temp_s0_2;
    void* temp_s0_3;
    u32* temp_s0_4;

    void* madr;

    u32* temp_s1;
    u16 chunk_sector_match;
    int audio_write_next;

    s32 write_idx;
    s32 read_idx;

    flag = 0;

    if (MOVIE_STATE->sectors_remaining == 0)
    {
        volatile MovieState* vms;
        u32* ptr_a;
        while (CdGetSector(&hdr, 8) == 0);

        vms = VOL_MOVIE_STATE;
        if (hdr[2] > ((u32)MOVIE_STATE->total_frames))
        {
            MOVIE_STATE->end_of_stream = 1;
            return 0;
        }

        vms->frame_number = hdr[2];

        if (((u16*)hdr)[2] != 0)
        {
            return 1;
        }

        if (((u16*)hdr)[1] == 0x8001)
        {

            write_idx = vms->video_write_idx;
            read_idx = vms->video_read_idx;

            if (((write_idx == read_idx) && (vms->last_video_frame == vms->last_consumed_video_frame)) ||
                ((write_idx != read_idx) && (read_idx < vms->video_write_idx)))
            {

                if (vms->video_ring_capacity < (vms->video_write_idx + ((u16*)hdr)[3]))
                {
                    if (read_idx >= ((s32)((u16*)hdr)[3]))
                    {
                        flag = 1;
                        vms->video_ring_size = (s32)vms->video_write_idx;
                        vms->video_write_idx = 0;
                    }
                }
                else
                {
                    flag = 1;
                }
            }
            else if (write_idx != read_idx)
            {
                if (read_idx >= (vms->video_write_idx + ((u16*)hdr)[3]))
                {
                    flag = 1;
                }
            }

            if (flag != 0)
            {

                s32 write_index;
                u8* sector_ptr;

                sector_ptr = (u8*)MOVIE_STATE->video_data_base + ((8 * MOVIE_STATE->video_write_idx) * 252);
                while (CdGetSector(sector_ptr, 0x1F8) == 0);

                ptr_a = hdr;

                write_index = MOVIE_STATE->video_write_idx;
                sector_ptr = (u8*)MOVIE_STATE->video_table_base + (write_index << 5);

                /* now sector_ptr is in s0 */
                ((u32*)sector_ptr)[0] = ptr_a[0];
                ((u32*)sector_ptr)[1] = ptr_a[1];
                ((u32*)sector_ptr)[2] = ptr_a[2];
                ((u32*)sector_ptr)[3] = ptr_a[3];
                ((u32*)sector_ptr)[4] = ptr_a[4];
                ((u32*)sector_ptr)[5] = ptr_a[5];
                ((u32*)sector_ptr)[6] = ptr_a[6];
                ((u32*)sector_ptr)[7] = ptr_a[7];

                MOVIE_STATE->sectors_remaining = (((u16*)hdr)[3]) - 1;
                if (!(MOVIE_STATE->sectors_remaining & 0xFFFF))
                {
                    u32 total_frames;

                    total_frames = (u32)MOVIE_STATE->total_frames;
                    VOL_MOVIE_STATE->video_write_idx = (s32)(VOL_MOVIE_STATE->video_write_idx + 1);

                    VOL_MOVIE_STATE->last_video_frame = VOL_MOVIE_STATE->frame_number;

                    has_more_frames = VOL_MOVIE_STATE->frame_number < total_frames;

                    goto block_49;
                }
                else
                {
                    MOVIE_STATE->continuation_type = 0;
                    MOVIE_STATE->chunk_sector_idx = 1U;
                }
            }
        }
        else
        {
            MovieState* ms;
            s32 audio_write_idx_l;
            audio_write_idx_l = vms->audio_write_idx;
            temp_a1_2 = vms->audio_read_idx;

            if (((audio_write_idx_l == temp_a1_2) && (vms->last_audio_frame == vms->last_consumed_audio_frame)) ||
                ((audio_write_idx_l != temp_a1_2) && (temp_a1_2 < vms->audio_write_idx)))
            {
                if (vms->audio_ring_capacity < (vms->audio_write_idx + ((u16*)hdr)[3]))
                {
                    if (temp_a1_2 >= ((s32)((u16*)hdr)[3]))
                    {
                        flag = 1;
                        vms->audio_ring_size = (s32)vms->audio_write_idx;
                        vms->audio_write_idx = 0;
                    }
                }
                else
                {
                    flag = 1;
                }
            }
            else if (temp_a1_2 >= (vms->audio_write_idx + ((u16*)hdr)[3]))
            {
                flag = 1;
            }

            if (flag != 0)
            {
                u8* sector_ptr;

                sector_ptr = ((u8*)MOVIE_STATE->audio_data_base + (VOL_MOVIE_STATE->audio_write_idx << 0xB)) + 0x20;
                while (CdGetSector(sector_ptr, 0x1F8) == 0);

                ptr_a = &hdr[0];
                temp_s0_2 = (u32*)(*MOVIE_STATE).audio_data_base;
                sector_ptr = (u32*)((u8*)MOVIE_STATE->audio_data_base + (VOL_MOVIE_STATE->audio_write_idx << 0xB));
                ((u32*)sector_ptr)[0] = ptr_a[0];
                ((u32*)sector_ptr)[1] = ptr_a[1];
                ((u32*)sector_ptr)[2] = ptr_a[2];
                ((u32*)sector_ptr)[3] = ptr_a[3];
                ((u32*)sector_ptr)[4] = ptr_a[4];
                ((u32*)sector_ptr)[5] = ptr_a[5];
                ((u32*)sector_ptr)[6] = ptr_a[6];
                ((u32*)sector_ptr)[7] = ptr_a[7];
                MOVIE_STATE->sectors_remaining = ((u16*)hdr)[3] - 1;
                if (!(MOVIE_STATE->sectors_remaining & 0xFFFF))
                {
                    VOL_MOVIE_STATE->audio_write_idx = (s32)(VOL_MOVIE_STATE->audio_write_idx + 1);
                    VOL_MOVIE_STATE->last_audio_frame = (u32)VOL_MOVIE_STATE->frame_number;

                    if (VOL_MOVIE_STATE->frame_number > ((u32)MOVIE_STATE->total_frames))
                    {
                        return 0;
                    }
                }
                else
                {
                    MOVIE_STATE->continuation_type = 1;
                    MOVIE_STATE->chunk_sector_idx = 1U;
                }
            }
            ms = MOVIE_STATE;
            if (g_audioStreamState == 1)
            {
                ms->unk92 = 2;
            }
            return 1;
        }
    }
    else if (MOVIE_STATE->continuation_type == 0)
    {

        temp_s1 = (u32*)((u8*)MOVIE_STATE->video_table_base +
                         ((VOL_MOVIE_STATE->video_write_idx + MOVIE_STATE->chunk_sector_idx) << 5));
        while (CdGetSector(temp_s1, 8) == 0);

        if (((((u16*)temp_s1)[1] == 0x8001) && (temp_s1[2] == MOVIE_STATE->frame_number)) &&
            (((u16*)temp_s1)[2] == MOVIE_STATE->chunk_sector_idx))
        {
            madr = (u8*)MOVIE_STATE->video_data_base +
                   ((2 * (VOL_MOVIE_STATE->video_write_idx + (MOVIE_STATE->chunk_sector_idx & 0xFFFF))) * 1008);
            while (CdGetSector(madr, 0x1F8) == 0);

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (!(MOVIE_STATE->sectors_remaining))
            {
                u32 offset;
                u32 total_frames;
                total_frames = (u32)MOVIE_STATE->total_frames;

                offset = 1;
                VOL_MOVIE_STATE->video_write_idx =
                    (s32)(MOVIE_STATE->chunk_sector_idx + (VOL_MOVIE_STATE->video_write_idx + offset));

                VOL_MOVIE_STATE->last_video_frame = VOL_MOVIE_STATE->frame_number;
                has_more_frames = ((u32*)temp_s1)[2] < total_frames;

            block_49:
                if (has_more_frames == 0)
                {

                    return 0;
                }
                has_more_frames = (u32)temp_s1[2];
                return 1;
            }
            goto block_64;
        }

        MOVIE_STATE->frame_number = (u32)temp_s1[2];
        MOVIE_STATE->sectors_remaining = 0U;

        if (((u32)MOVIE_STATE->total_frames) > ((u32)temp_s1[2]))
        {
            return 1;
        }

        MOVIE_STATE->end_of_stream = 1;
        return 0;
    }
    else
    {

        madr = (u8*)MOVIE_STATE->audio_data_base +
               ((VOL_MOVIE_STATE->audio_write_idx + MOVIE_STATE->chunk_sector_idx) << 0xB);
        while (CdGetSector(madr, 8) == 0);

        if (((((u16*)temp_s1)[1] == 1) && (temp_s1[2] == MOVIE_STATE->frame_number)) &&
            ((chunk_sector_match = (MOVIE_STATE->chunk_sector_idx)) == (temp_s1[1] & 0xFFFFu)))
        {
            madr = ((u8*)MOVIE_STATE->audio_data_base +
                    ((VOL_MOVIE_STATE->audio_write_idx + (MOVIE_STATE->chunk_sector_idx & 0xFFFF)) << 0xB)) +
                   0x20;
            while (CdGetSector(madr, 0x1F8) == 0);

            // I think I need to use a variable for MOVIE_STATE in order to remove the reload in the else block
            // however, doing this puts it in a saved register possibly due to the goto reference?

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (!(MOVIE_STATE->sectors_remaining & 0xFFFF))
            {
                audio_write_next = VOL_MOVIE_STATE->audio_write_idx + 1;
                VOL_MOVIE_STATE->audio_write_idx = (s32)(audio_write_next + MOVIE_STATE->chunk_sector_idx);

                MOVIE_STATE->last_audio_frame = VOL_MOVIE_STATE->frame_number;
                if (((u32)temp_s1[2]) > ((u32)MOVIE_STATE->total_frames))
                {
                    return 0;

                block_64:
                    MOVIE_STATE->chunk_sector_idx = (u16)(MOVIE_STATE->chunk_sector_idx + 1);
                }
                return 1;
            }
        }

        MOVIE_STATE->frame_number = (u32)temp_s1[2];
        MOVIE_STATE->sectors_remaining = 0U;
        if (((u32)temp_s1[2]) <= ((u32)MOVIE_STATE->total_frames))
        {
            return 1;
        }

        MOVIE_STATE->end_of_stream = 1;
        return 0;
    }

    return 1;
}

/**
 * @brief Find the next unqueued audio ring entry and return its address.
 *
 * Walks the audio ring forward from audio_read_idx skipping the
 * already-buffered entries, wrapping at the ring size. Charges the entry's
 * sector count to @ref MovieState::audio_buffered_count.
 *
 * @param out_entry Receives a pointer to the entry's 2048-byte CD sector
 *                 in @ref MovieState::audio_data_base. Untouched if no entry
 *                 is available.
 *
 * @return 1 if an entry was produced, 0 if the ring is empty or fully queued.
 *
 * @see https://decomp.me/scratch/I2Ddr (100%)
 */
static s32 get_next_audio_entry(AudioSector** out_entry)
{
    s32 next_idx;
    AudioSector* entry;

    /* Return immediately if nothing is available: ring empty and no secondary data pending */
    if ((MOVIE_STATE->audio_write_idx == MOVIE_STATE->audio_read_idx) &&
        (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
    {
        return 0;
    }

    /* Wrap read_idx back to 0 when it reaches the end of the ring */
    if ((VOL_MOVIE_STATE->audio_write_idx <= MOVIE_STATE->audio_read_idx) &&
        (MOVIE_STATE->audio_read_idx == MOVIE_STATE->audio_ring_size))
    {
        MOVIE_STATE->audio_read_idx = 0;

        if (MOVIE_STATE->audio_write_idx == 0 &&
            (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
        {
            return 0;
        }
    }

    /* Look past already-buffered entries to find the next one to queue */
    next_idx = MOVIE_STATE->audio_read_idx + MOVIE_STATE->audio_buffered_count;

    /* Wrap next_idx if it overflows the ring */
    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_idx >= VOL_MOVIE_STATE->audio_ring_size))
    {
        next_idx -= MOVIE_STATE->audio_ring_size;
    }

    /* All loaded entries are already queued; nothing new to dispatch */
    if ((next_idx == MOVIE_STATE->audio_write_idx) && (MOVIE_STATE->audio_buffered_count != 0))
    {
        return 0;
    }

    /* Resolve entry: each entry occupies one 2048-byte CD sector in the audio data buffer */
    entry = &MOVIE_STATE->audio_data_base[next_idx];
    MOVIE_STATE->audio_buffered_count += entry->header.sector_count;
    *out_entry = entry;
    return 1;
}

/**
 * @brief DrawSync completion callback; flushes deferred VRAM upload / MDEC submit.
 *
 * If the GPU is idle (g_busy == 0), services @ref MovieState::pending_vram_upload
 * (LoadImage + schedule next decode) and @ref MovieState::pending_mdec_decode
 * (DecDCTout for the staged buffer).
 *
 * @see https://decomp.me/scratch/TApbR (100%)
 */
static void draw_sync_callback(void)
{
    s16 width;
    s16 height;
    volatile MovieState* movie_state = VOL_MOVIE_STATE;

    if (g_busy != 0)
    {
        return;
    }

    movie_state->draw_sync_target = 0;

    if (movie_state->pending_vram_upload != 0)
    {

        LoadImage(&MOVIE_STATE->rects[2], MOVIE_STATE->mdec_output_buf[movie_state->out_buf_idx]);
        movie_schedule_next_decode();
        movie_state->pending_vram_upload = 0;
    }

    if (movie_state->pending_mdec_decode != 0)
    {
        width = movie_state->rects[2].w;
        height = movie_state->rects[2].h;

        DecDCTout(MOVIE_STATE->mdec_output_buf[movie_state->out_buf_idx], (width * height) / 2);
        movie_state->pending_mdec_decode = 0;
    }
}

/**
 * @brief Resolve pointers to the next available video ring entry.
 *
 * Wraps video_read_idx if it has reached video_ring_size. On success returns
 * pointers to the entry's VLC data buffer and 32-byte header.
 *
 * @param out_vlc_data     Receives the 2016-byte VLC data buffer pointer.
 * @param out_entry_header Receives the 32-byte entry header pointer.
 *
 * @return 1 if an entry is available, 0 otherwise (ring empty and last frame consumed).
 *
 * @see https://decomp.me/scratch/OJvsJ (100%)
 */
static s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header)
{
    s32 read_idx;
    s32 write_idx;

    if ((MOVIE_STATE->video_write_idx == MOVIE_STATE->video_read_idx) &&
        (MOVIE_STATE->last_video_frame == MOVIE_STATE->last_consumed_video_frame))
    {
        return 0;
    }

    write_idx = VOL_MOVIE_STATE->video_write_idx;
    read_idx = VOL_MOVIE_STATE->video_read_idx;

    if ((read_idx >= write_idx) && (read_idx == MOVIE_STATE->video_ring_size))
    {
        MOVIE_STATE->video_read_idx = 0;

        if ((MOVIE_STATE->video_write_idx == 0) &&
            (MOVIE_STATE->last_video_frame == MOVIE_STATE->last_consumed_video_frame))
        {
            return 0;
        }
    }

    *out_entry_header = &MOVIE_STATE->video_table_base[MOVIE_STATE->video_read_idx];
    *out_vlc_data = &MOVIE_STATE->video_data_base[MOVIE_STATE->video_read_idx];
    return 1;
}

/**
 * @brief Advance video_read_idx past the entry currently being consumed.
 *
 * Reads sector_count from the entry header to step the index, wraps to 0
 * when the new index hits video_ring_size, and records the consumed frame
 * number in @ref MovieState::last_consumed_video_frame.
 *
 * @see https://decomp.me/scratch/SUBK5 (100%)
 */
static void advance_video_read(void)
{
    SectorEntry* entry;
    s32 next_index;

    entry = &MOVIE_STATE->video_table_base[MOVIE_STATE->video_read_idx].header;
    next_index = MOVIE_STATE->video_read_idx + entry->sector_count;

    if ((MOVIE_STATE->video_read_idx >= MOVIE_STATE->video_write_idx) && (next_index == MOVIE_STATE->video_ring_size))
    {
        next_index = 0;
    }

    MOVIE_STATE->last_consumed_video_frame = entry->frame_number;
    MOVIE_STATE->video_read_idx = next_index;
}

/**
 * @brief Advance audio_read_idx past the entry currently being consumed.
 *
 * Reads sector_count from the entry header to step the index, decrements
 * @ref MovieState::audio_buffered_count, wraps to 0 when the new index hits
 * video_ring_size (note: not audio_ring_size - see note below), and records
 * the consumed frame number in @ref MovieState::last_consumed_audio_frame.
 *
 * @note Comparing the audio next_index against video_ring_size (not
 *       audio_ring_size) appears to be an original-game bug; preserved
 *       verbatim to keep the asm matching.
 *
 * @see https://decomp.me/scratch/6Xjsu (100%)
 */
static void advance_audio_read(void)
{
    SectorEntry* entry;
    s32 next_index;

    entry = &MOVIE_STATE->audio_data_base[MOVIE_STATE->audio_read_idx].header;
    next_index = MOVIE_STATE->audio_read_idx + entry->sector_count;

    MOVIE_STATE->audio_buffered_count = MOVIE_STATE->audio_buffered_count - entry->sector_count;

    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_index == MOVIE_STATE->video_ring_size))
    {
        next_index = 0;
    }

    MOVIE_STATE->last_consumed_audio_frame = entry->frame_number;
    MOVIE_STATE->audio_read_idx = next_index;
}