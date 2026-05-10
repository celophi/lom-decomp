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
 * @param movieIndex Cinematic to play (0..4). Indices outside this range
 *                   fall through to the case-4 default (898 frames).
 *
 * @see https://decomp.me/scratch/gkEWm (100%)
 */
void movie_play(s32 movieIndex)
{
    DISPENV env[2];
    DISPENV* pDispEnv;
    volatile MovieState* state;
    s32 audioFadeVol;
    s32 retryLimit;
    char endStateMatch;
    s32 error_status;
    s32 timeout;
    unsigned short new_var;
    u16 buttons;
    s32 frameCount;
    u32 idx;
    s32 resourceIdx;
    s32 initFlags;

    VSync(0);
    func_800157DC();
    func_800157B0(1);
    VSync(0);
    func_800157DC();
    cdrom_process_state();
    /* Pre-playback skip gate: if user is already holding a skip button on
     * movie 0 when we get here, bail before staging the stream. */
    if ((((movieIndex & 0xFFFF) == 0) && ((SCD_REGS)->deviceState < SCD_DEVICE_STATE_OK)) &&
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
     * Five MDEC cinematics; movieIndex selects one (0..4). The frame count
     * matches each movie's BS stream length and is used by movie_init to set
     * the movieInitFlags stop condition. The CD resource index is the per-movie
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

    switch ((u16)(movieIndex & 0xFFFF))
    {
    case 0:
        frameCount = 2098;
        initFlags = 0x80;
        break;

    case 1:
        frameCount = 2473;
        initFlags = 0x80;
        break;

    case 2:
        frameCount = 1318;
        initFlags = 0x80;
        break;

    case 3:
        frameCount = 5368;
        initFlags = 0x80;
        break;

    case 4:

    default:
        frameCount = 898;
        initFlags = 0x80;
        break;
    }

    resourceIdx = (movieIndex & 0xFFFF) + 0x16A0;
    movie_init(resourceIdx, initFlags, frameCount, 0);
    VSync(0);
    func_800157DC();
    audioFadeVol = AUDIO_FADE_DISARMED;
    retryLimit = 5;
    state = (MovieState*)0x801ED500;
    endStateMatch = END_STATE_DONE;

    while (TRUE)
    {
        error_status = cdrom_get_error_status();

        while ((error_status != 0) && (error_status != retryLimit))
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

                if (state->endState == endStateMatch)
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
        new_var = (u16)(movieIndex & 0xFFFF);
        VSync(0);
        pDispEnv = &env[0];
        if (state->chunkIdx == 0)
        {
            pDispEnv = &env[1];
        }
        PutDispEnv(pDispEnv);
        SetDispMask(1);
        func_800157DC();
        cdrom_process_state();

        idx = new_var;
        if ((idx < MOVIE_SKIPPABLE_MAX) && ((SCD_REGS)->deviceState < SCD_DEVICE_STATE_OK))
        {
            buttons = (SCD_REGS)->unk4;
            if (((idx != 0) ? ((buttons & MOVIE1_SKIP_MASK) != 0) : ((buttons & MOVIE0_SKIP_MASK) != 0)) != 0)
            {
                if (g_cdAudioReady == 0)
                {
                    break;
                }

                if (audioFadeVol == AUDIO_FADE_DISARMED)
                {
                    audioFadeVol = AUDIO_FADE_INITIAL;
                }
            }
        }

        if ((g_cdAudioReady != 0) && (audioFadeVol != AUDIO_FADE_DISARMED))
        {
            func_80023030(audioFadeVol);

            if (audioFadeVol == 0)
            {
                break;
            }

            audioFadeVol -= AUDIO_FADE_STEP;
        }

        if (state->endState == endStateMatch)
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
 * @ref g_gpuMode (standard vs BreakDraw/dynamic-allocBase).
 *
 * @param resourceIndex CD resource id of the BS stream (e.g. 0x16A0..0x16A4).
 * @param flags         Bit 0..6 → MovieState.gpuMode. Bit 7 → MovieState.interlaceMode
 *                      (despite the name, this selects the *audio source*, not
 *                      GPU interlacing — see @note below).
 * @param totalFrames   Frame-count stop condition; set into MovieState.totalFrames.
 * @param initBufferIdx Initial active chunk index (0 or 1). Path B uses it to
 *                      seed rects[2] from rects[initBufferIdx].
 *
 * @note `MovieState.gpuMode` and the `g_gpuMode` global are *the same byte* at
 *       0x801ED590 (offset 0x90 in MovieState). The write at the top of this
 *       function and the `if (g_gpuMode == 0)` branch read the same storage.
 *
 * @note In shipped play, `movie_play` always passes flags = 0x80, so
 *       gpuMode = 0 (path A taken every time) and interlaceMode = 1 (XA audio).
 *       Path B is dead in production; preserved for matching, which is also why
 *       its read of an uninitialised rects[0].x (`>= 0x300` guard) is inert.
 *
 * @note `interlaceMode` controls audio source, not display interlacing:
 *         1 → akao_cmd_e8_start_xa_stream + cd_volume   (real CD-XA streaming)
 *         0 → akao_cmd_c8(0x7FFF) + xa_setup_panning    (SPU/synth path)
 *
 * @note Path-A memory map (RAM addresses are literals in this function):
 *           0x80147000  videoTableBase    (50 × 32   = 0x640)
 *           0x80147640  videoDataBase     (50 × 2016)
 *           0x80160000  audioDataBase     (16 × 2048 = 0x8000)
 *           0x80168000  vlcTable          (0x11000)
 *           0x80179000  vlcInputBuf[0]    (0x14000)
 *           0x8018D000  vlcInputBuf[1]    (0x14000)
 *           0x801A1000  mdecOutputBuf[0]  (0x2D00 = 24 × 240 × 2)
 *           0x801A3D00  mdecOutputBuf[1]  (0x2D00)
 *       Sizes match rects[2] = 24-wide × 240-tall macroblock decode column.
 *
 * @note Path-B memory map (relative to AllocInfo::allocBase):
 *           0x80147000           videoTableBase   (30 × 32 = 0x3C0)
 *           videoTableBase+0x3C0  videoDataBase
 *           0x80156000           audioDataBase
 *           0x8015E000           vlcInputBuf[0]   (0x11000)
 *           0x8016F000           vlcInputBuf[1]
 *           allocBase            vlcTable
 *           allocBase+0x11000    mdecOutputBuf[0] (0x1E00 = 16 × 240 × 2)
 *           allocBase+0x12E00    mdecOutputBuf[1]
 *       rects[2] is 16 wide here.
 *
 * @see https://decomp.me/scratch/g5PtA (91.61%)
 * @see https://decomp.me/scratch/ICOiP (incorrect but better match)
 */
void movie_init(s32 resourceIndex, s32 flags, s32 totalFrames, s32 initBufferIdx)
{
    MovieState* movie_state;
    AllocInfo* allocInfo = g_allocInfo;
    short value;
    short value2;
    void* addr;
    unsigned short new_var;

    MOVIE_STATE->gpuMode = (s8)(flags & 0x7F);
    if (flags & 0x80)
    {
        MOVIE_STATE->interlaceMode = 1;
    }
    else
    {
        MOVIE_STATE->interlaceMode = 0;
    }

    if (MOVIE_STATE->gpuMode == 0)
    {

        MOVIE_STATE->videoTableBase = (u32*)0x80147000;
        MOVIE_STATE->audioDataBase = (AudioSector*)0x80160000;
        MOVIE_STATE->vlcTable = (u32*)0x80168000;
        MOVIE_STATE->vlcInputBuf[0] = (u32*)0x80179000;
        MOVIE_STATE->vlcInputBuf[1] = (u_long*)0x8018D000;
        MOVIE_STATE->mdecOutputBuf[0] = (u_long*)0x801A1000;
        MOVIE_STATE->mdecOutputBuf[1] = (VideoVlcPayload*)0x801A3D00;

        MOVIE_STATE->rects[1].y = 0xF0;
        MOVIE_STATE->rects[2].h = 0xF0;
        MOVIE_STATE->rects[1].h = 0xF0;

        MOVIE_STATE->videoRingCapacity = 0x32;

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

        MOVIE_STATE->audioRingCapacity = 0x10;
        MOVIE_STATE->videoDataBase = (void*)0x80147640;
        MOVIE_STATE->chunkIdx = 0;
    }
    else
    {
        MOVIE_STATE->videoTableBase = (VideoSectorEntry*)0x80147000;
        MOVIE_STATE->audioDataBase = (AudioSector*)0x80156000;
        MOVIE_STATE->vlcInputBuf[0] = (u8*)0x8015E000;
        MOVIE_STATE->vlcInputBuf[1] = (u8*)0x8016F000;
        MOVIE_STATE->vlcTable = (u32)allocInfo->allocBase;
        MOVIE_STATE->mdecOutputBuf[0] = (u_long*)(allocInfo->allocBase + 0x11000);
        MOVIE_STATE->mdecOutputBuf[1] = (u_long*)(allocInfo->allocBase + 0x12E00);

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
        MOVIE_STATE->rects[2].x = MOVIE_STATE->rects[initBufferIdx].x;
        new_var = (unsigned short)((MovieState*)0x801ED500)->rects[initBufferIdx].y;
        MOVIE_STATE->rects[2].w = 0x10;
        MOVIE_STATE->videoRingCapacity = 0x1E;

        MOVIE_STATE->audioRingCapacity = 0x10;

        MOVIE_STATE->videoDataBase = (VideoVlcPayload*)((u32)MOVIE_STATE->videoTableBase + 0x3C0);
        MOVIE_STATE->chunkIdx = (s8)initBufferIdx;
        ((MovieState*)0x801ED500)->rects[2].y = new_var;
    }

    MOVIE_STATE->resourceIndex = resourceIndex;
    MOVIE_STATE->currentFrame = 0;
    MOVIE_STATE->totalFrames = totalFrames;
    MOVIE_STATE->inputBufIdx = 0;
    MOVIE_STATE->vlcRetryCount = 0;
    MOVIE_STATE->mdecRetryPending = 0;
    MOVIE_STATE->busy = 0;
    MOVIE_STATE->draw_sync_target = 0;
    MOVIE_STATE->outBufIdx = 0;
    MOVIE_STATE->pending_vram_upload = 0;
    MOVIE_STATE->pending_mdec_decode = 0;
    MOVIE_STATE->mdecBusy = 0;
    MOVIE_STATE->frame_ready = 0;
    MOVIE_STATE->endOfStream = 0;
    MOVIE_STATE->endState = END_STATE_RUNNING;
    MOVIE_STATE->unk92 = 0;

    MOVIE_STATE->videoWriteIdx = 0;
    MOVIE_STATE->videoReadIdx = 0;
    MOVIE_STATE->videoRingSize = 0;
    MOVIE_STATE->audioWriteIdx = 0;
    MOVIE_STATE->audioReadIdx = 0;
    MOVIE_STATE->audioRingSize = 0;
    MOVIE_STATE->audioBufferedCount = 0;
    MOVIE_STATE->frameNumber = 0;
    MOVIE_STATE->continuationType = 0;

    MOVIE_STATE->sectorsRemaining = 0;
    MOVIE_STATE->lastVideoFrame = (u32)(-1);
    MOVIE_STATE->lastConsumedVideoFrame = (u32)(-1);
    MOVIE_STATE->lastAudioFrame = (u32)(-1);
    MOVIE_STATE->lastConsumedAudioFrame = (u32)(-1);

    /* Psy-Q's DecDCToutCallback takes a single function pointer; the trailing
     * p1/p2/p3 are codegen scratch — they pin specific values into $a1/$a2/$a3
     * at the call site to reproduce the original register state, and are
     * ignored by the callee. Path A: (vlcInputBuf[1], vlcInputBuf[0], vlcTable).
     * Path B: ((u16)rects[initBufferIdx].y, 0x11000, initBufferIdx). */
    MOVIE_STATE->decDCToutCallback = (u32)DecDCToutCallback(&movie_mdec_out_callback);
    MOVIE_STATE->drawSyncCallback = DrawSyncCallback(&draw_sync_callback);

    if (MOVIE_STATE->interlaceMode != 0)
    {
        akao_cmd_e8_start_xa_stream((u32)MOVIE_STATE->audioDataBase, (u32)(MOVIE_STATE->audioRingCapacity << 0xB));
        akao_cmd_e4_set_cd_volume(0x7F);
    }
    else
    {
        akao_cmd_c8(0x7FFF);
        akao_xa_setup_panning(0xA0);
    }

    cdrom_wait_queue_empty();
    if (!((MovieState*)0x801ED500)->audioDataBase)
    {
    }
    cdrom_queue_command(CdlReadS, (s16)resourceIndex, NULL, &movie_cd_sector_callback);

    if (g_gpuMode == 0)
    {
        VSync(0);
        SetDispMask(0);
        ClearImage(&MOVIE_STATE->rects[0], 0, 0, 0);
        ClearImage(&MOVIE_STATE->rects[1], 0, 0, 0);
        DecDCTvlcBuild((u_short*)MOVIE_STATE->vlcTable);
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
 * @see https://decomp.me/scratch/xXKIt (98.86%)
 */
void movie_update(void)
{
    long audioCapacity;             /* audioRingCapacity reload */
    MovieState* stateAlias;         /* sequencing alias used after totalFrames check */
    int field9DZeroFlag;            /* (frame_ready == 0) flag / sign-shift temp */
    VideoVlcPayload* vlc_payload;   /* raw bitstream for the next video frame */
    VideoSectorEntry* entry_header; /* 32-byte sector header for the same frame */
    AudioSector* audio_entry;       /* next audio ring entry (header + payload) */
    MovieState* mdecAlias;          /* sequencing alias used in MDEC retry block */
    s32 tmp = 0;                    /* vlc-complete flag, then audioBufferedCount compare */
    MovieState* combined = MOVIE_STATE;
    int wordCount;              /* DCT word count temp / zero literal */
    volatile int audioFrameNum; /* frame number from latest audio entry */
    if (g_mdecRetryPending != 0)
    {
        mdecAlias = combined;
        if ((mdecAlias->mdecBusy == 0) && (combined->frame_ready == 0))
        {
            combined->mdecBusy = 1;
            DecDCTin((u_long*)((MovieState*)combined)->vlcInputBuf[combined->inputBufIdx],
                     (combined->gpuMode & 0xFFFFu) == 0);
            {
                s32 temp = ((s16)((MovieState*)combined)->rects[2].w) * ((s16)((MovieState*)combined)->rects[2].h);
                wordCount = temp + (((unsigned)temp) >> 31);
                DecDCTout((((MovieState*)combined)->mdecOutputBuf[combined->outBufIdx]), wordCount >> 1);
            }
            combined->mdecRetryPending = 0;
        }
    }
    if ((g_mdecRetryPending == 0) & 0xFFFFu)
    {

        {
            u8 v0 = MOVIE_STATE->vlcRetryCount;
            if (v0 != 0)
            {
                v0--;
                MOVIE_STATE->vlcRetryCount = v0;
                if ((v0 & 0xFF) == 0)
                {
                    DecDCTvlcSize2(0);
                }
                if (DecDCTvlc2(0, 0, (DECDCTTAB*)MOVIE_STATE->vlcTable) == 0)
                {
                    tmp = 1;
                    MOVIE_STATE->vlcRetryCount = 0;
                }
            }
            else if (get_next_video_entry(&vlc_payload, &entry_header) != 0)
            {
                MOVIE_STATE->currentFrame = entry_header->header.frameNumber;
                stateAlias = MOVIE_STATE;
                if ((entry_header->header.frameNumber >= MOVIE_STATE->totalFrames) &&
                    (stateAlias->endState == END_STATE_RUNNING))
                {
                    MOVIE_STATE->endState = END_STATE_NEAR_END;
                }
                {
                    int one;
                    MOVIE_STATE->inputBufIdx = 1 - MOVIE_STATE->inputBufIdx;
                }
                if (MOVIE_STATE->gpuMode == 0)
                {
                    DecDCTvlcSize2(0x1000);
                    MOVIE_STATE->vlcRetryCount = 3;
                }
                else
                {
                    DecDCTvlcSize2(0x16AA);
                    MOVIE_STATE->vlcRetryCount = 1;
                }
                if (DecDCTvlc2((u_long*)vlc_payload, (u_long*)MOVIE_STATE->vlcInputBuf[MOVIE_STATE->inputBufIdx],
                               (DECDCTTAB*)MOVIE_STATE->vlcTable) == 0)
                {
                    tmp = 1;
                    MOVIE_STATE->vlcRetryCount = 0;
                }
            }
            else
            {
                if (((!entry_header) && (!entry_header)) && (!entry_header))
                {
                }
                if ((MOVIE_STATE->endOfStream != 0) && (MOVIE_STATE->mdecBusy == 0))
                {
                    MOVIE_STATE->endState = END_STATE_DONE;
                }
            }
        }
    }
    wordCount = 0;
    if (tmp != wordCount)
    {
        advance_video_read();
        combined = MOVIE_STATE;
        if ((combined->mdecBusy == wordCount) && (field9DZeroFlag = combined->frame_ready == wordCount))
        {
            combined->mdecBusy = 1;
            DecDCTin((u_long*)((MovieState*)combined)->vlcInputBuf[combined->inputBufIdx], combined->gpuMode == 0);
            {
                s32 temp = ((s16)((MovieState*)combined)->rects[2].w) * ((s16)((MovieState*)combined)->rects[2].h);
                field9DZeroFlag = ((unsigned)temp) >> 31;
                DecDCTout((((MovieState*)combined)->mdecOutputBuf[combined->outBufIdx]), (temp + field9DZeroFlag) >> 1);
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
        if (get_next_audio_entry(&audio_entry) != 0)
        {
            audioFrameNum = (combined->currentFrame = audio_entry->header.frameNumber);
            if ((audioFrameNum > combined->totalFrames) && (combined->endState < END_STATE_DONE))
            {
                combined->endState = END_STATE_DONE;
            }
            akao_xa_advance_frame(audioFrameNum);
        }
        combined = MOVIE_STATE;
        if (g_audioStreamState == 2)
        {
            combined = MOVIE_STATE;
            audioCapacity = combined->audioRingCapacity;
            tmp = combined->audioBufferedCount;
            if (tmp >= ((s32)(audioCapacity >> 1)))
            {
                akao_cmd_98_9a_9c_9e(3);
                combined->unk92 = 0;
            }
        }
        combined = MOVIE_STATE;
        if ((combined->audioWriteIdx != MOVIE_STATE->audioReadIdx) ||
            (MOVIE_STATE->lastAudioFrame != MOVIE_STATE->lastConsumedAudioFrame))
        {
            s32 tmp = akao_xa_get_position();
            if (((tmp != (-1)) && (MOVIE_STATE->audioBufferedCount != 0)) &&
                (MOVIE_STATE->audioReadIdx != ((u32)(tmp * 2))))
            {
                advance_audio_read();
            }
            do
            {
            } while (0);
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
    int new_var;

    if (g_gpuMode == 0)
    {
        if (((u8)g_cdStatusByte3) == 1)
        {
            cdrom_verify_recovery();
        }
        temp = DrawSync(1);
        if (temp < 2)
        {
            LoadImage((RECT*)0x801ED530, base->mdecOutputBuf[base->outBufIdx]);
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
        new_var = 0;
        if (temp != (-1))
        {
            LoadImage2((RECT*)0x801ED530, base->mdecOutputBuf[base->outBufIdx]);
            if (temp != new_var)
            {
                DrawOTag((u_long*)temp);
            }
            bp_high = (MovieState*)0x801e0000;
        }
        else
        {
            LoadImage((RECT*)0x801ED530, base->mdecOutputBuf[base->outBufIdx]);
        }
    }

    if (MOVIE_STATE->pending_vram_upload == new_var)
    {
        movie_schedule_next_decode();
        return;
    }
    MOVIE_STATE->mdecBusy = 1;
}

/**
 * @brief Advance the decode position and schedule the next MDEC decode.
 *
 * Steps the rect[2] frame position by its width. If still inside the current
 * chunk, kicks off DecDCTout immediately (or sets @ref MovieState::pending_mdec_decode
 * if the GPU is busy). Otherwise toggles to the other chunk, resets the frame
 * position, and signals @ref MovieState::frame_ready.
 *
 * @see https://decomp.me/scratch/E7XCZ (98.72%)
 */
void movie_schedule_next_decode(void)
{
    /* NOTE: the volatile u8/u16 casts on every access force lbu/lhu, so the
     * underlying field signedness is irrelevant. */
    MovieState* ptr = MOVIE_STATE;
    unsigned short nextOutBufIdx;
    u16 curFramePos;
    u16 frameStep;
    u16 newFramePos;
    u16* new_var;
    s32 newFramePosSigned;
    s32 chunkEnd;
    s16 a;
    s16 c;
    u32 decodeSize;
    int decodeWordCount;
    nextOutBufIdx = 1 - (*((volatile u8*)(&ptr->outBufIdx)));
    curFramePos = *((volatile u16*)(&ptr->rects[2].x));
    frameStep = *((volatile u16*)(&ptr->rects[2].w));
    newFramePos = curFramePos + frameStep;
    *((volatile u16*)(&ptr->rects[2].x)) = newFramePos;
    newFramePosSigned = (s16)newFramePos;
    *((volatile u8*)(&ptr->outBufIdx)) = nextOutBufIdx;
    a = ptr->rects[*((volatile u8*)(&ptr->chunkIdx))].x;
    c = ptr->rects[*((volatile u8*)(&ptr->chunkIdx))].w;
    chunkEnd = a + c;
    if (newFramePosSigned < chunkEnd)
    {
        if ((*((volatile u8*)(&ptr->draw_sync_target))) < 2U)
        {
            decodeSize = ((s16)frameStep) * ((s16)(*((volatile u16*)(&ptr->rects[2].h))));
            decodeWordCount = ((int)(decodeSize + (decodeSize >> 31))) >> 1;
            DecDCTout(ptr->mdecOutputBuf[*((volatile u8*)(&ptr->outBufIdx))], decodeWordCount);
            *((volatile u8*)(&ptr->mdecBusy)) = 2;
        }
        else
        {
            *((volatile u8*)(&ptr->mdecBusy)) = 1;
            *((volatile u8*)(&ptr->pending_mdec_decode)) = 1;
        }
    }
    else
    {
        /* advance to the next chunk and reset the frame position */
        *((volatile u8*)(&ptr->chunkIdx)) = 1 - (*((volatile u8*)(&ptr->chunkIdx)));
        ptr->rects[2].x = ptr->rects[*((volatile u8*)(&ptr->chunkIdx))].x;
        ptr->rects[2].y = *(new_var = (u16*)&ptr->rects[*((volatile u8*)(&ptr->chunkIdx))].y);
        *((volatile u8*)(&ptr->frame_ready)) = 1;
        *((volatile u8*)(&ptr->mdecBusy)) = 0;
        if ((*((volatile u8*)(&ptr->endState))) == END_STATE_NEAR_END)
        {
            *((volatile u8*)(&ptr->endState)) = END_STATE_DONE;
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
 * gpuMode selects the transfer path:
 *   - 0: wait for DrawSync then use LoadImage (standard DMA).
 *   - non-zero: interrupt the current draw via BreakDraw then use LoadImage2.
 *
 * @note The s8 status-byte fields (busy/draw_sync_target) emit `lb` where the
 *       original asm used `lbu`. Currently 93.87% non-matching; revisit the
 *       field types (s8 → u8) if the percentage regresses further.
 *
 * @see https://decomp.me/scratch/JTTFr (93.87%)
 */
void movie_service_video_ops(void)
{
    volatile MovieState* G = (volatile MovieState*)0x801ED500;
    int wordCount;
    u_long* breakDrawResult;
    if (!G->pending_vram_upload)
    {
        if (!G->pending_mdec_decode)
        {
            return;
        }
    }
    if (G->gpuMode == 0)
    {
        /* Standard path: wait until the GPU has drained before uploading */
        if (DrawSync(1) >= 2)
        {
            return;
        }
        if (G->pending_vram_upload)
        {
            u8 t = G->pending_vram_upload;
            if (t)
            {
                G->busy = 1;
                t = G->outBufIdx;
                LoadImage((RECT*)0x801ED530, G->mdecOutputBuf[t]);
                G->draw_sync_target = DrawSync(1) + 1;
                G->pending_vram_upload = 0;
                movie_schedule_next_decode();
            }
            G->busy = 0;
        }
        G = (volatile MovieState*)0x801ED500;
        if (G->pending_mdec_decode)
        {
            u8 t = G->pending_mdec_decode;
            if (t)
            {
                s32 temp;
                G->busy = 1;
                /* word count = (width * height) / 2, rounded toward zero for signed values */
                temp = ((s32)G->rects[2].w) * ((s32)G->rects[2].h);
                wordCount = temp + (((u32)temp) >> 31);
                DecDCTout(G->mdecOutputBuf[G->outBufIdx], wordCount >> 1);
                G->pending_mdec_decode = 0;
            }
            G->busy = 0;
        }
    }
    else // <-- changed block starts here
    {
        /* BreakDraw path: interrupt the current draw primitive list to upload immediately */
        if (G->pending_vram_upload)
        {
            u8 t = G->pending_vram_upload;
            if (t)
            {
                s32 bd;
                G->busy = 1;
                breakDrawResult = BreakDraw();
                bd = (s32)breakDrawResult;
                if (bd != (-1))
                {
                    LoadImage2((RECT*)0x801ED530, G->mdecOutputBuf[G->outBufIdx]);
                    if (bd != 0)
                    {
                        /* Resume the interrupted OTag list */
                        DrawOTag((u_long*)bd);
                    }
                    movie_schedule_next_decode();
                    G->pending_vram_upload = 0;
                }
            }
        }
        g_busy = 0; // <-- moved outside the inner if
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
 * @see https://decomp.me/scratch/HptYe (76.45%)
 */
s32 movie_cd_sector_callback(void)
{
    SectorBuffer hdr; /* 32-byte sector header (8 u32 words) read from CD */
    s32 should_load;
    u32 count;
    u16 remaining;
    void* dest;
    u32* entry;
    s32 next_idx;

    should_load = 0;

    if (MOVIE_STATE->sectorsRemaining == 0) /* same as VOL_MOVIE_STATE->sectorsRemaining, but accessed directly */
    {
        while (CdGetSector(hdr, 8) == 0);

        if (hdr[2] > VOL_MOVIE_STATE->totalFrames)
        {
            VOL_MOVIE_STATE->endOfStream = 1;
            return 0;
        }

        VOL_MOVIE_STATE->frameNumber = hdr[2];

        /* sub-sector index (offset 4) must be 0 for a header sector */
        if (((u16*)hdr)[2] != 0)
        {
            return 1;
        }

        /* sector type (offset 2): 0x8001 = video, otherwise audio */
        if (((u16*)hdr)[1] == 0x8001)
        {
            if (VOL_MOVIE_STATE->videoWriteIdx == VOL_MOVIE_STATE->videoReadIdx)
            {
                if (VOL_MOVIE_STATE->lastVideoFrame == VOL_MOVIE_STATE->lastConsumedVideoFrame)
                    goto video_cap;
                goto video_done;
            }
            if (VOL_MOVIE_STATE->videoReadIdx >= VOL_MOVIE_STATE->videoWriteIdx)
                goto video_else;
        video_cap:
            count = ((u16*)hdr)[3];
            if (VOL_MOVIE_STATE->videoRingCapacity < VOL_MOVIE_STATE->videoWriteIdx + (s32)count)
            {
                if (VOL_MOVIE_STATE->videoReadIdx >= (s32)count)
                {
                    should_load = 1;
                    VOL_MOVIE_STATE->videoRingSize = VOL_MOVIE_STATE->videoWriteIdx;
                    VOL_MOVIE_STATE->videoWriteIdx = 0;
                }
            }
            else
            {
                should_load = 1;
            }
            goto video_done;
        video_else:
            count = ((u16*)hdr)[3];
            if (VOL_MOVIE_STATE->videoReadIdx >= VOL_MOVIE_STATE->videoWriteIdx + (s32)count)
                should_load = 1;
        video_done:;

            if (should_load != 0)
            {
                dest = &VOL_MOVIE_STATE->videoDataBase[VOL_MOVIE_STATE->videoWriteIdx];
                while (CdGetSector(dest, 0x1F8) == 0)
                {
                }

                /* Copy the full 32-byte CD header verbatim into the table entry. */
                entry = (u32*)&VOL_MOVIE_STATE->videoTableBase[VOL_MOVIE_STATE->videoWriteIdx];
                entry[0] = hdr[0];
                entry[1] = hdr[1];
                entry[2] = hdr[2];
                entry[3] = hdr[3];
                entry[4] = hdr[4];
                entry[5] = hdr[5];
                entry[6] = hdr[6];
                entry[7] = hdr[7];

                remaining = (u16)(((u16*)hdr)[3] - 1);
                VOL_MOVIE_STATE->sectorsRemaining = remaining;
                if (remaining == 0)
                {
                    VOL_MOVIE_STATE->videoWriteIdx += 1;
                    VOL_MOVIE_STATE->lastVideoFrame = VOL_MOVIE_STATE->frameNumber;
                    return (VOL_MOVIE_STATE->frameNumber < VOL_MOVIE_STATE->totalFrames) ? 1 : 0;
                }
                VOL_MOVIE_STATE->continuationType = 0;
                VOL_MOVIE_STATE->chunkSectorIdx = 1;
                return 1;
            }
            return 1;
        }

        /* audio sector */
        if (VOL_MOVIE_STATE->audioWriteIdx == VOL_MOVIE_STATE->audioReadIdx)
        {
            if (VOL_MOVIE_STATE->lastAudioFrame == VOL_MOVIE_STATE->lastConsumedAudioFrame)
                goto audio_cap;
            goto audio_done;
        }
        if (VOL_MOVIE_STATE->audioReadIdx >= VOL_MOVIE_STATE->audioWriteIdx)
            goto audio_else;
    audio_cap:
        count = ((u16*)hdr)[3];
        if (VOL_MOVIE_STATE->audioRingCapacity < VOL_MOVIE_STATE->audioWriteIdx + (s32)count)
        {
            if (VOL_MOVIE_STATE->audioReadIdx >= (s32)count)
            {
                should_load = 1;
                VOL_MOVIE_STATE->audioRingSize = VOL_MOVIE_STATE->audioWriteIdx;
                VOL_MOVIE_STATE->audioWriteIdx = 0;
            }
        }
        else
        {
            should_load = 1;
        }
        goto audio_done;
    audio_else:
        count = ((u16*)hdr)[3];
        if (VOL_MOVIE_STATE->audioReadIdx >= VOL_MOVIE_STATE->audioWriteIdx + (s32)count)
            should_load = 1;
    audio_done:;

        if (should_load != 0)
        {
            dest = VOL_MOVIE_STATE->audioDataBase[VOL_MOVIE_STATE->audioWriteIdx].payload;
            while (CdGetSector(dest, 0x1F8) == 0)
            {
            }

            entry = (u32*)&VOL_MOVIE_STATE->audioDataBase[VOL_MOVIE_STATE->audioWriteIdx];
            entry[0] = hdr[0];
            entry[1] = hdr[1];
            entry[2] = hdr[2];
            entry[3] = hdr[3];
            entry[4] = hdr[4];
            entry[5] = hdr[5];
            entry[6] = hdr[6];
            entry[7] = hdr[7];

            remaining = (u16)(((u16*)hdr)[3] - 1);
            VOL_MOVIE_STATE->sectorsRemaining = remaining;
            if (remaining == 0)
            {
                VOL_MOVIE_STATE->audioWriteIdx += 1;
                VOL_MOVIE_STATE->lastAudioFrame = VOL_MOVIE_STATE->frameNumber;
                if (VOL_MOVIE_STATE->totalFrames < VOL_MOVIE_STATE->frameNumber)
                    return 0;
            }
            else
            {
                VOL_MOVIE_STATE->continuationType = 1;
                VOL_MOVIE_STATE->chunkSectorIdx = 1;
            }
        }

        if (g_audioStreamState == 1) /* reads MovieState.unk92 directly */
        {
            VOL_MOVIE_STATE->unk92 = 2;
        }
        return 1;
    }

    /* D_801ED57E != 0 — reading a continuation sector for the same frame */
    if (VOL_MOVIE_STATE->continuationType == 0)
    {
        /* video continuation */
        for (;;)
        {
            next_idx = VOL_MOVIE_STATE->videoWriteIdx + VOL_MOVIE_STATE->chunkSectorIdx;
            entry = (u32*)&VOL_MOVIE_STATE->videoTableBase[next_idx];
            while (CdGetSector(entry, 8) == 0)
            {
            }

            if (((u16*)entry)[1] == 0x8001 && entry[2] == VOL_MOVIE_STATE->frameNumber &&
                ((u16*)entry)[2] == VOL_MOVIE_STATE->chunkSectorIdx)
            {
                dest = &VOL_MOVIE_STATE->videoDataBase[next_idx];
                while (CdGetSector(dest, 0x1F8) == 0)
                {
                }

                remaining = VOL_MOVIE_STATE->sectorsRemaining - 1;
                VOL_MOVIE_STATE->sectorsRemaining = remaining;
                if (remaining != 0)
                {
                    VOL_MOVIE_STATE->chunkSectorIdx += 1;
                    return 1;
                }
                VOL_MOVIE_STATE->videoWriteIdx = VOL_MOVIE_STATE->videoWriteIdx + 1 + VOL_MOVIE_STATE->chunkSectorIdx;
                VOL_MOVIE_STATE->lastVideoFrame = VOL_MOVIE_STATE->frameNumber;
                return (entry[2] < VOL_MOVIE_STATE->totalFrames) ? 1 : 0;
            }
            VOL_MOVIE_STATE->sectorsRemaining = 0;
            VOL_MOVIE_STATE->frameNumber = entry[2];
            if (entry[2] < VOL_MOVIE_STATE->totalFrames)
                break;
            VOL_MOVIE_STATE->endOfStream = 1;
            return 0;
        }
        return 1;
    }

    /* audio continuation */
    for (;;)
    {
        next_idx = VOL_MOVIE_STATE->audioWriteIdx + VOL_MOVIE_STATE->chunkSectorIdx;
        entry = (u32*)&VOL_MOVIE_STATE->audioDataBase[next_idx];
        while (CdGetSector(entry, 8) == 0)
        {
        }

        if (((u16*)entry)[1] == 1 && entry[2] == VOL_MOVIE_STATE->frameNumber &&
            ((u16*)entry)[2] == VOL_MOVIE_STATE->chunkSectorIdx)
        {
            dest = VOL_MOVIE_STATE->audioDataBase[next_idx].payload;
            while (CdGetSector(dest, 0x1F8) == 0)
            {
            }

            remaining = VOL_MOVIE_STATE->sectorsRemaining - 1;
            VOL_MOVIE_STATE->sectorsRemaining = remaining;
            if (remaining != 0)
            {
                VOL_MOVIE_STATE->chunkSectorIdx += 1;
                return 1;
            }
            VOL_MOVIE_STATE->audioWriteIdx = VOL_MOVIE_STATE->audioWriteIdx + 1 + VOL_MOVIE_STATE->chunkSectorIdx;
            VOL_MOVIE_STATE->lastAudioFrame = VOL_MOVIE_STATE->frameNumber;
            if (VOL_MOVIE_STATE->totalFrames < entry[2])
                return 0;
            return 1;
        }
        VOL_MOVIE_STATE->sectorsRemaining = 0;
        VOL_MOVIE_STATE->frameNumber = entry[2];
        if (!(VOL_MOVIE_STATE->totalFrames < entry[2]))
            break;
        VOL_MOVIE_STATE->endOfStream = 1;
        return 0;
    }
    return 1;
}

/**
 * @brief Find the next unqueued audio ring entry and return its address.
 *
 * Walks the audio ring forward from audioReadIdx skipping the
 * already-buffered entries, wrapping at the ring size. Charges the entry's
 * sector count to @ref MovieState::audioBufferedCount.
 *
 * @param outEntry Receives a pointer to the entry's 2048-byte CD sector
 *                 in @ref MovieState::audioDataBase. Untouched if no entry
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
    if ((MOVIE_STATE->audioWriteIdx == MOVIE_STATE->audioReadIdx) &&
        (MOVIE_STATE->lastAudioFrame == MOVIE_STATE->lastConsumedAudioFrame))
    {
        return 0;
    }

    /* Wrap readIdx back to 0 when it reaches the end of the ring */
    if ((VOL_MOVIE_STATE->audioWriteIdx <= MOVIE_STATE->audioReadIdx) &&
        (MOVIE_STATE->audioReadIdx == MOVIE_STATE->audioRingSize))
    {
        MOVIE_STATE->audioReadIdx = 0;

        if (MOVIE_STATE->audioWriteIdx == 0 && (MOVIE_STATE->lastAudioFrame == MOVIE_STATE->lastConsumedAudioFrame))
        {
            return 0;
        }
    }

    /* Look past already-buffered entries to find the next one to queue */
    next_idx = MOVIE_STATE->audioReadIdx + MOVIE_STATE->audioBufferedCount;

    /* Wrap next_idx if it overflows the ring */
    if ((MOVIE_STATE->audioReadIdx >= MOVIE_STATE->audioWriteIdx) && (next_idx >= VOL_MOVIE_STATE->audioRingSize))
    {
        next_idx -= MOVIE_STATE->audioRingSize;
    }

    /* All loaded entries are already queued; nothing new to dispatch */
    if ((next_idx == MOVIE_STATE->audioWriteIdx) && (MOVIE_STATE->audioBufferedCount != 0))
    {
        return 0;
    }

    /* Resolve entry: each entry occupies one 2048-byte CD sector in the audio data buffer */
    entry = &MOVIE_STATE->audioDataBase[next_idx];
    MOVIE_STATE->audioBufferedCount += entry->header.sectorCount;
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

        LoadImage(&MOVIE_STATE->rects[2], MOVIE_STATE->mdecOutputBuf[movie_state->outBufIdx]);
        movie_schedule_next_decode();
        movie_state->pending_vram_upload = 0;
    }

    if (movie_state->pending_mdec_decode != 0)
    {
        width = movie_state->rects[2].w;
        height = movie_state->rects[2].h;

        DecDCTout(MOVIE_STATE->mdecOutputBuf[movie_state->outBufIdx], (width * height) / 2);
        movie_state->pending_mdec_decode = 0;
    }
}

/**
 * @brief Resolve pointers to the next available video ring entry.
 *
 * Wraps videoReadIdx if it has reached videoRingSize. On success returns
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

    if ((MOVIE_STATE->videoWriteIdx == MOVIE_STATE->videoReadIdx) &&
        (MOVIE_STATE->lastVideoFrame == MOVIE_STATE->lastConsumedVideoFrame))
    {
        return 0;
    }

    write_idx = VOL_MOVIE_STATE->videoWriteIdx;
    read_idx = VOL_MOVIE_STATE->videoReadIdx;

    if ((read_idx >= write_idx) && (read_idx == MOVIE_STATE->videoRingSize))
    {
        MOVIE_STATE->videoReadIdx = 0;

        if ((MOVIE_STATE->videoWriteIdx == 0) && (MOVIE_STATE->lastVideoFrame == MOVIE_STATE->lastConsumedVideoFrame))
        {
            return 0;
        }
    }

    *out_entry_header = &MOVIE_STATE->videoTableBase[MOVIE_STATE->videoReadIdx];
    *out_vlc_data = &MOVIE_STATE->videoDataBase[MOVIE_STATE->videoReadIdx];
    return 1;
}

/**
 * @brief Advance videoReadIdx past the entry currently being consumed.
 *
 * Reads sectorCount from the entry header to step the index, wraps to 0
 * when the new index hits videoRingSize, and records the consumed frame
 * number in @ref MovieState::lastConsumedVideoFrame.
 *
 * @see https://decomp.me/scratch/SUBK5 (100%)
 */
static void advance_video_read(void)
{
    SectorEntry* entry;
    s32 next_index;

    entry = &MOVIE_STATE->videoTableBase[MOVIE_STATE->videoReadIdx].header;
    next_index = MOVIE_STATE->videoReadIdx + entry->sectorCount;

    if ((MOVIE_STATE->videoReadIdx >= MOVIE_STATE->videoWriteIdx) && (next_index == MOVIE_STATE->videoRingSize))
    {
        next_index = 0;
    }

    MOVIE_STATE->lastConsumedVideoFrame = entry->frameNumber;
    MOVIE_STATE->videoReadIdx = next_index;
}

/**
 * @brief Advance audioReadIdx past the entry currently being consumed.
 *
 * Reads sectorCount from the entry header to step the index, decrements
 * @ref MovieState::audioBufferedCount, wraps to 0 when the new index hits
 * videoRingSize (note: not audioRingSize — see note below), and records
 * the consumed frame number in @ref MovieState::lastConsumedAudioFrame.
 *
 * @note Comparing the audio nextIndex against videoRingSize (not
 *       audioRingSize) appears to be an original-game bug; preserved
 *       verbatim to keep the asm matching.
 *
 * @see https://decomp.me/scratch/6Xjsu (100%)
 */
static void advance_audio_read(void)
{
    SectorEntry* entry;
    s32 next_index;

    entry = &MOVIE_STATE->audioDataBase[MOVIE_STATE->audioReadIdx].header;
    next_index = MOVIE_STATE->audioReadIdx + entry->sectorCount;

    MOVIE_STATE->audioBufferedCount = MOVIE_STATE->audioBufferedCount - entry->sectorCount;

    if ((MOVIE_STATE->audioReadIdx >= MOVIE_STATE->audioWriteIdx) && (next_index == MOVIE_STATE->videoRingSize))
    {
        next_index = 0;
    }

    MOVIE_STATE->lastConsumedAudioFrame = entry->frameNumber;
    MOVIE_STATE->audioReadIdx = next_index;
}