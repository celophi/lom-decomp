#include "movie.h"

/*
 * NOTE: every "struct" in movie.h that begins at offset 0 is a different
 * named view of the SAME runtime block at 0x801ED500. The split exists
 * because each function was decompiled in isolation on decomp.me and tuned
 * against its own field layout. Do NOT try to unify these types without
 * re-verifying every offset — the matching builds depend on those exact
 * offsets being preserved per-function.
 */

/**
 * decomp.me link (97.51%) https://decomp.me/scratch/gkEWm
 * this one is a WIP without gotos (https://decomp.me/scratch/Gq1vj)
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
    u32 frameCount;
    VSync(0);
    func_800157DC();
    func_800157B0(1);
    VSync(0);
    func_800157DC();
    cdrom_process_state();
    if ((((movieIndex & 0xFFFF) == 0) && (((SRC_801ED600*)0x801ED600)->unk0 < 3)) &&
        ((((SRC_801ED600*)0x801ED600)->unk2 & 0xFF0F) != 0))
    {
        return;
    }
    func_800158E0();
    DecDCTReset(0);
    timeout = 0xF0;
    SetDefDispEnv(&env[0], 0, 0, 320, timeout);
    SetDefDispEnv(&env[1], 0, timeout, 320, timeout);
    env[0].isrgb24 = (env[1].isrgb24 = 1);

    /*
     * Five MDEC cinematics; movieIndex selects one (0..4). The frame count
     * matches each movie's BS stream length and is used by movie_init to set
     * the totalFrames stop condition. The CD resource index is the per-movie
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
        break;

    case 1:
        frameCount = 2473;
        break;

    case 2:
        frameCount = 1318;
        break;

    case 3:
        frameCount = 5368;
        break;

    case 4:

    default:
        frameCount = 898;
        break;
    }

    movie_init((movieIndex & 0xFFFF) + 0x16A0, 0x80, frameCount, 0);
    VSync(0);
    func_800157DC();
    audioFadeVol = -1;
    retryLimit = 5;
    state = (MovieState*)0x801ED500;
    endStateMatch = 2;
    do
    {
        error_status = cdrom_get_error_status();
        while ((error_status != 0) && (((((error_status & 0xFFu) & 0xFFu) & 0xFFu) & 0xFFu) != retryLimit))
        {
            func_800157B0(1);
            VSync(0);
            func_800157DC();
            cdrom_process_state();
            error_status = cdrom_get_error_status();
        }

        timeout = 0x2000;
        while (state->field9D == 0)
        {
            do
            {
                movie_update();
                if (state->field9D != 0)
                {
                    break;
                }
                if (state->endState == endStateMatch)
                {
                    break;
                }
                movie_service_video_ops();
            } while ((--timeout) != 0);
            if (timeout == 0)
            {
                cdrom_process_state();
            }
            timeout = 0x2000;
        }

        state->field9D = 0;
        func_800157B0(4);
        new_var = (u32)(movieIndex & 0xFFFF);
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
        {
            u32 a0 = new_var;
            if ((a0 < 2) && (((SRC_801ED600*)0x801ED600)->unk0 < 3))
            {
                u16 val = ((SRC_801ED600*)0x801ED600)->unk4;
                if (((a0 != 0) ? ((val & ((0, 0x400A))) != 0) : ((val & 0xFF0F) != 0)) != 0)
                {
                    if (g_cdAudioReady == 0)
                    {
                        break;
                    }
                    if (audioFadeVol == (-1))
                    {
                        audioFadeVol = 0x70;
                    }
                }
            }
        }
        if (((g_cdAudioReady != 0) && (audioFadeVol != (-1))) != 0)
        {
            func_80023030(audioFadeVol);
            if (audioFadeVol == 0)
            {
                break;
            }
            audioFadeVol -= 0x10;
        }
    } while (state->endState != endStateMatch);
    func_800158E0();
    cdrom_reset();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}

/**
 * decomp.me link (91.61%) https://decomp.me/scratch/hR71L
 * incorrect but better match https://decomp.me/scratch/ICOiP
 */
void movie_init(s32 resourceIndex, s32 flags, s32 totalFrames, s32 initBufferIdx)
{
    u32 p1;
    u8* vlcTablePtr;        /* base of VLC code table */
    MovieState* stateRef2;  /* used after VSync; alias of 0x801ED500 */
    MovieState* stateRef3;  /* used as ClearImage arg base */
    u32 p2;
    MovieState* stateRef4;  /* sequencing alias from rect setup */
    u32 p3;
    MovieState* stateRef5;  /* sequencing alias from rect copies */
    u8* vlcTablePtr2;       /* duplicate of vlcTablePtr (forces an extra move) */
    u8** videoTableBasePtr; /* &state->videoTableBase reload */
    int stateAddrInt;       /* literal 0x801ED500 used for vlcTable store */
    u8** videoTableBaseRef; /* &state->videoTableBase (NOT a DISPENV) */
    AllocInfo* allocInfo = g_allocInfo;

    MOVIE_STATE->gpuMode = (s8)(flags & 0x7F);
    if (flags & 0x80)
    {
        MOVIE_STATE->interlaceMode = 1;
    }
    else
    {
        MOVIE_STATE->interlaceMode = 0;
    }

    videoTableBaseRef = &MOVIE_STATE->videoTableBase;

    if (g_gpuMode == 0)
    {
        p3 = 0x80168000;
        p2 = 0x80179000;
        vlcTablePtr = (u8*)0x80147000;
        p1 = 0x8018D000;

        MOVIE_STATE->videoTableBase = vlcTablePtr;
        p2++;
        p2--;
        MOVIE_STATE->mdecOutputBuf[1] = (u8*)0x801A3D00;
        MOVIE_STATE->audioDataBase = (u8*)0x80160000;
        MOVIE_STATE->lastConsumedVideoFrame = (u32)(-1);
        MOVIE_STATE->rects[1].w = 0x1E0;
        (stateRef4 = MOVIE_STATE)->rects[0].w = 0x1E0;
        MOVIE_STATE->rects[2].w = 0x18;
        MOVIE_STATE->videoRingCapacity = 0x32;
        stateAddrInt = 0x801ED500;
        MOVIE_STATE->vlcInputBuf[0] = (u8*)p2;
        MOVIE_STATE->vlcInputBuf[1] = (u8*)p1;
        MOVIE_STATE->mdecOutputBuf[0] = (u8*)0x801A1000;
        MOVIE_STATE->rects[0].x = p3 * 0;
        MOVIE_STATE->rects[1].x = 0;
        MOVIE_STATE->rects[0].y = 0;
        MOVIE_STATE->rects[1].y = 0xF0;
        MOVIE_STATE->rects[2].h = 0xF0;
        MOVIE_STATE->rects[1].h = 0xF0;
        MOVIE_STATE->rects[0].h = 0xF0;
        MOVIE_STATE->rects[2].x = 0;
        MOVIE_STATE->rects[2].y = 0;
        MOVIE_STATE->audioRingCapacity = 0x10;
        MOVIE_STATE->videoDataBase = (u8*)0x80147640;
        MOVIE_STATE->chunkIdx = 0;
        ((MovieState*)stateAddrInt)->vlcTable = p3;
    }
    else
    {
        p2 = 0x11000;
        p3 = initBufferIdx;
        MOVIE_STATE->videoTableBase = (u8*)0x80147000;
        MOVIE_STATE->audioDataBase = (u8*)0x80156000;
        vlcTablePtr = (u8*)allocInfo->allocBase;
        MOVIE_STATE->vlcInputBuf[0] = (u8*)0x8015E000;
        MOVIE_STATE->vlcInputBuf[1] = (u8*)0x8016F000;
        videoTableBasePtr = &(*videoTableBaseRef);
        vlcTablePtr2 = vlcTablePtr;
        MOVIE_STATE->vlcTable = vlcTablePtr2;
        MOVIE_STATE->mdecOutputBuf[0] = (u8*)(allocInfo->allocBase + p2);
        MOVIE_STATE->mdecOutputBuf[1] = (u8*)(allocInfo->allocBase + 0x12E00);

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

        stateRef5 = MOVIE_STATE;
        stateRef5->rects[1].w = stateRef5->rects[0].w;
        {
            stateRef5->rects[1].h = stateRef5->rects[0].h;
            stateRef5->rects[2].h = stateRef5->rects[0].h;
            stateRef5->rects[2].x = (&stateRef5->rects[initBufferIdx])->x;
            // FIX: cast to unsigned short to force zero-extension (lhu) instead of sign-extension (lh)
            p1 = (unsigned short)stateRef5->rects[initBufferIdx].y;
        }
        stateRef5->rects[2].w = 0x10;
        stateRef5->videoRingCapacity = 0x1E;
        MOVIE_STATE->audioRingCapacity = 0x10;
        MOVIE_STATE->chunkIdx = (s8)initBufferIdx;
        MOVIE_STATE->videoDataBase = (u8*)(((u32)(*videoTableBasePtr)) + 0x3C0);
        MOVIE_STATE->rects[2].y = p1;
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
    MOVIE_STATE->field9D = 0;
    MOVIE_STATE->endOfStream = 0;
    MOVIE_STATE->endState = 0;
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

    MOVIE_STATE->decDCToutCallback = (u32)DecDCToutCallback(&movie_mdec_out_callback, p1, p2, p3);
    MOVIE_STATE->drawSyncCallback = DrawSyncCallback(&movie_draw_sync_callback);

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
    stateRef2 = MOVIE_STATE;
    cdrom_queue_command(CdlReadS, (s16)resourceIndex, NULL, &movie_cd_sector_callback);

    if (g_gpuMode == 0)
    {
        VSync(p2 = 0);
        SetDispMask(p2);
        ClearImage(&(stateRef3 = stateRef2)->rects[p2], 0, p2, 0);
        ClearImage(&MOVIE_STATE->rects[1], p2, 0, p2);
        DecDCTvlcBuild((u_short*)stateRef2->vlcTable);
        DrawSync(0);
    }
}

/**
 * decomp.me link (98.86%) https://decomp.me/scratch/xXKIt
 */
void movie_update(void)
{
    long audioCapacity;     /* audioRingCapacity reload */
    MovieState* stateAlias; /* sequencing alias used after totalFrames check */
    int field9DZeroFlag;    /* (field9D == 0) flag / sign-shift temp */
    void* hdr;
    void* sp14;
    MovieState* mdecAlias;      /* sequencing alias used in MDEC retry block */
    s32 tmp = 0;                /* vlc-complete flag, then audioBufferedCount compare */
    MovieState* combined = MOVIE_STATE;
    int wordCount;              /* DCT word count temp / zero literal */
    volatile int audioFrameNum; /* frame number from latest audio entry */
    if (g_mdecRetryPending != 0)
    {
        mdecAlias = combined;
        if ((mdecAlias->mdecBusy == 0) && (combined->field9D == 0))
        {
            combined->mdecBusy = 1;
            DecDCTin((u_long*)((MovieState*)combined)->vlcInputBuf[combined->inputBufIdx],
                     (combined->gpuMode & 0xFFFFu) == 0);
            {
                s32 temp = ((s16)((MovieState*)combined)->rects[2].w) * ((s16)((MovieState*)combined)->rects[2].h);
                wordCount = temp + (((unsigned)temp) >> 31);
                DecDCTout((u_long*)(((MovieState*)combined)->mdecOutputBuf[combined->outBufIdx]), wordCount >> 1);
            }
            combined->mdecRetryPending = 0;
        }
    }
    if ((g_mdecRetryPending == 0) & 0xFFFFu)
    {
        ;
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
            else if (movie_get_next_video_entry(&hdr, &sp14) != 0)
            {
                MOVIE_STATE->currentFrame = ((u32*)sp14)[2];
                stateAlias = MOVIE_STATE;
                if ((((u32*)sp14)[2] >= MOVIE_STATE->totalFrames) && (stateAlias->endState == 0))
                {
                    MOVIE_STATE->endState = 1;
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
                if (DecDCTvlc2((u_long*)hdr, (u_long*)MOVIE_STATE->vlcInputBuf[MOVIE_STATE->inputBufIdx],
                               (DECDCTTAB*)MOVIE_STATE->vlcTable) == 0)
                {
                    tmp = 1;
                    MOVIE_STATE->vlcRetryCount = 0;
                }
            }
            else
            {
                if (((!sp14) && (!sp14)) && (!sp14))
                {
                }
                if ((MOVIE_STATE->endOfStream != 0) && (MOVIE_STATE->mdecBusy == 0))
                {
                    MOVIE_STATE->endState = 2;
                }
            }
        }
    }
    wordCount = 0;
    if (tmp != wordCount)
    {
        movie_advance_video_read();
        combined = MOVIE_STATE;
        if ((combined->mdecBusy == wordCount) && (field9DZeroFlag = combined->field9D == wordCount))
        {
            combined->mdecBusy = 1;
            DecDCTin((u_long*)((MovieState*)combined)->vlcInputBuf[combined->inputBufIdx], combined->gpuMode == 0);
            {
                s32 temp = ((s16)((MovieState*)combined)->rects[2].w) * ((s16)((MovieState*)combined)->rects[2].h);
                field9DZeroFlag = ((unsigned)temp) >> 31;
                DecDCTout((u_long*)(((MovieState*)combined)->mdecOutputBuf[combined->outBufIdx]),
                          (temp + field9DZeroFlag) >> 1);
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
        if (movie_get_next_audio_entry(&hdr) != 0)
        {
            audioFrameNum = (combined->currentFrame = ((u32*)hdr)[2]);
            if ((audioFrameNum > combined->totalFrames) && (combined->endState < 2))
            {
                combined->endState = 2;
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
                func_8002246C(3);
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
                movie_advance_audio_read(tmp);
            }
            do
            {
            } while (0);
        }
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/HVkZ6
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
            LoadImage((RECT*)0x801ED530, (u_long*)base->mdecOutputBuf[base->outBufIdx]);
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
            LoadImage2((RECT*)0x801ED530, (u_long*)base->mdecOutputBuf[base->outBufIdx]);
            if (temp != new_var)
            {
                DrawOTag((u_long*)temp);
            }
            bp_high = (MovieState*)0x801e0000;
        }
        else
        {
            LoadImage((RECT*)0x801ED530, (u_long*)base->mdecOutputBuf[base->outBufIdx]);
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
 * decomp.me: (98.72%) https://decomp.me/scratch/E7XCZ
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
            DecDCTout((u32*)ptr->mdecOutputBuf[*((volatile u8*)(&ptr->outBufIdx))], decodeWordCount);
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
        *((volatile u8*)(&ptr->field9D)) = 1;
        *((volatile u8*)(&ptr->mdecBusy)) = 0;
        if ((*((volatile u8*)(&ptr->endState))) == 1)
        {
            *((volatile u8*)(&ptr->endState)) = 2;
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
                LoadImage((RECT*)0x801ED530, (u_long*)G->mdecOutputBuf[t]);
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
                DecDCTout((u_long*)G->mdecOutputBuf[G->outBufIdx], wordCount >> 1);
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
                    LoadImage2((RECT*)0x801ED530, (u_long*)G->mdecOutputBuf[G->outBufIdx]);
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
 * decomp.me (76.45%) https://decomp.me/scratch/HptYe
 *
 * NOTE: previously contained references to bogus field names `audioDataBase4`
 * and `audioDataBaseC`; both fixed to `lastConsumedVideoFrame` /
 * `lastConsumedAudioFrame` (offsets 0x84 / 0x8C) as part of the MovieState
 * merge — the surrounding ring-empty logic is "writeIdx == readIdx AND last
 * produced == last consumed".
 *
 * CD sector-arrival callback, called by the CD-ROM interrupt once per sector read.
 *
 * Reads the 8-word (32-byte) sector header, determines if the sector is video
 * (type 0x8001) or audio, checks whether the corresponding ring buffer has room,
 * then copies the 504-word (2016-byte) payload into the buffer and updates the
 * write index.  Multi-sector frames are handled via g_sectorsRemaining: when 0 this is
 * the first (header) sector; when non-zero we are reading continuation sectors
 * for the same frame and skip the ring-capacity check.
 *
 * Returns 1 to keep streaming, 0 when the stream has ended or should pause.
 */
s32 movie_cd_sector_callback(void)
{
    SectorBuffer hdr; /* 32-byte sector header (8 u32 words) read from CD */
    s32 do_load;      /* s0 in assembly */
    volatile MovieState* const gp = MOVIE_STATE;
    u16* hdr16; /* u16 view of hdr for field access by word index */
    u32 count;
    u16 rem;
    void* dest;
    void* entry;

    do_load = 0;

    if (g_sectorsRemaining == 0)
    {
        /* read first header sector */
        while (CdGetSector(hdr, 8) == 0)
        {
        }

        if (gp->totalFrames < hdr[2])
        {
            gp->endOfStream = 1;
            return 0;
        }

        gp->frameNumber = hdr[2];

        /* check low word of hdr[1] (offset 0x14) */
        if (((u16*)hdr)[2] != 0)
            return 1;

        /* check high word of hdr[0] (offset 0x12) */
        if (((u16*)hdr)[1] == 0x8001)
        {
            /* 0x8001 sector type */
            if (gp->videoWriteIdx == gp->videoReadIdx)
            {
                if (gp->lastVideoFrame == gp->lastConsumedVideoFrame)
                {
                    count = ((u16*)hdr)[3];
                    if (gp->videoRingCapacity < gp->videoWriteIdx + (s32)count)
                    {
                        if (gp->videoReadIdx >= (s32)count)
                        {
                            do_load = 1;
                            gp->videoRingSize = gp->videoWriteIdx;
                            gp->videoWriteIdx = 0;
                        }
                    }
                    else
                    {
                        do_load = 1;
                    }
                }
            }
            else if (gp->videoReadIdx < gp->videoWriteIdx)
            {
                count = ((u16*)hdr)[3];
                if (gp->videoRingCapacity < gp->videoWriteIdx + (s32)count)
                {
                    if (gp->videoReadIdx >= (s32)count)
                    {
                        do_load = 1;
                        gp->videoRingSize = gp->videoWriteIdx;
                        gp->videoWriteIdx = 0;
                    }
                }
                else
                {
                    do_load = 1;
                }
            }
            else
            {
                count = ((u16*)hdr)[3];
                if (gp->videoReadIdx >= gp->videoWriteIdx + (s32)count)
                {
                    do_load = 1;
                }
            }

            if (do_load != 0)
            {
                dest = (void*)(gp->videoDataBase + (gp->videoWriteIdx * 2016));
                while (CdGetSector(dest, 0x1F8) == 0)
                {
                }

                entry = (void*)(gp->videoTableBase + (gp->videoWriteIdx << 5));
                ((u32*)entry)[0] = hdr[0];
                ((u32*)entry)[1] = hdr[1];
                ((u32*)entry)[2] = hdr[2];
                ((u32*)entry)[3] = hdr[3];
                ((u32*)entry)[4] = hdr[4];
                ((u32*)entry)[5] = hdr[5];
                ((u32*)entry)[6] = hdr[6];
                ((u32*)entry)[7] = hdr[7];

                rem = (u16)(((u16*)hdr)[3] - 1);
                gp->sectorsRemaining = rem;
                if (rem == 0)
                {
                    gp->videoWriteIdx += 1;
                    gp->lastVideoFrame = gp->frameNumber;
                    return (gp->frameNumber < gp->totalFrames) ? 1 : 0;
                }
                else
                {
                    gp->continuationType = 0;
                    gp->chunkSectorIdx = rem;
                    return 1;
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            /* other sector type */
            if (gp->audioWriteIdx == gp->audioReadIdx)
            {
                if (gp->lastAudioFrame == gp->lastConsumedAudioFrame)
                {
                    count = ((u16*)hdr)[3];
                    if (gp->audioRingCapacity < gp->audioWriteIdx + (s32)count)
                    {
                        if (gp->audioReadIdx >= (s32)count)
                        {
                            do_load = 1;
                            gp->audioRingSize = gp->audioWriteIdx;
                            gp->audioWriteIdx = 0;
                        }
                    }
                    else
                    {
                        do_load = 1;
                    }
                }
            }
            else if (gp->audioReadIdx < gp->audioWriteIdx)
            {
                count = ((u16*)hdr)[3];
                if (gp->audioRingCapacity < gp->audioWriteIdx + (s32)count)
                {
                    if (gp->audioReadIdx >= (s32)count)
                    {
                        do_load = 1;
                        gp->audioRingSize = gp->audioWriteIdx;
                        gp->audioWriteIdx = 0;
                    }
                }
                else
                {
                    do_load = 1;
                }
            }
            else
            {
                count = ((u16*)hdr)[3];
                if (gp->audioReadIdx >= gp->audioWriteIdx + (s32)count)
                {
                    do_load = 1;
                }
            }

            if (do_load != 0)
            {
                dest = (void*)(gp->audioDataBase + (gp->audioWriteIdx << 11) + 0x20);
                while (CdGetSector(dest, 0x1F8) == 0)
                {
                }

                entry = (void*)(gp->audioDataBase + (gp->audioWriteIdx << 11));
                ((u32*)entry)[0] = hdr[0];
                ((u32*)entry)[1] = hdr[1];
                ((u32*)entry)[2] = hdr[2];
                ((u32*)entry)[3] = hdr[3];
                ((u32*)entry)[4] = hdr[4];
                ((u32*)entry)[5] = hdr[5];
                ((u32*)entry)[6] = hdr[6];
                ((u32*)entry)[7] = hdr[7];

                rem = (u16)(((u16*)hdr)[3] - 1);
                gp->sectorsRemaining = rem;
                if (rem == 0)
                {
                    gp->audioWriteIdx += 1;
                    gp->lastAudioFrame = gp->frameNumber;
                    if (gp->totalFrames < gp->frameNumber)
                        return 0;
                }
                else
                {
                    gp->continuationType = rem;
                    gp->chunkSectorIdx = rem;
                }
            }

            if (g_audioStreamState == 1)
            {
                gp->unk92 = 2;
            }
            return 1;
        }
    }
    else
    {
        /* g_sectorsRemaining != 0 */
        if (gp->continuationType == 0)
        {
            for (;;)
            {
                entry = (void*)(gp->videoTableBase + ((gp->videoWriteIdx + gp->chunkSectorIdx) << 5));
                while (CdGetSector(entry, 8) == 0)
                {
                }

                hdr16 = (u16*)entry;
                if (hdr16[1] == 0x8001 && ((u32*)entry)[2] == gp->frameNumber && hdr16[2] == gp->chunkSectorIdx)
                {
                    dest = (void*)(gp->videoDataBase + ((gp->videoWriteIdx + gp->chunkSectorIdx) * 2016));
                    while (CdGetSector(dest, 0x1F8) == 0)
                    {
                    }

                    rem = gp->sectorsRemaining - 1;
                    gp->sectorsRemaining = rem;
                    if (rem != 0)
                    {
                        gp->chunkSectorIdx += 1;
                        return 1;
                    }
                    gp->videoWriteIdx = gp->videoWriteIdx + 1 + gp->chunkSectorIdx;
                    gp->lastVideoFrame = gp->frameNumber;
                    return (((u32*)entry)[2] < gp->totalFrames) ? 1 : 0;
                }
                gp->sectorsRemaining = 0;
                gp->frameNumber = ((u32*)entry)[2];
                if (((u32*)entry)[2] < gp->totalFrames)
                    break;
                gp->endOfStream = 1;
                return 0;
            }
            return 1;
        }
        else
        {
            for (;;)
            {
                entry = (void*)(gp->audioDataBase + ((gp->audioWriteIdx + gp->chunkSectorIdx) << 11));
                while (CdGetSector(entry, 8) == 0)
                {
                }

                hdr16 = (u16*)entry;
                if (hdr16[1] == 1 && ((u32*)entry)[2] == gp->frameNumber && hdr16[2] == gp->chunkSectorIdx)
                {
                    dest = (void*)(gp->audioDataBase + ((gp->audioWriteIdx + gp->chunkSectorIdx) << 11) + 0x20);
                    while (CdGetSector(dest, 0x1F8) == 0)
                    {
                    }

                    rem = gp->sectorsRemaining - 1;
                    gp->sectorsRemaining = rem;
                    if (rem != 0)
                    {
                        gp->chunkSectorIdx += 1;
                        return 1;
                    }
                    gp->audioWriteIdx = gp->audioWriteIdx + 1 + gp->chunkSectorIdx;
                    gp->lastAudioFrame = gp->frameNumber;
                    if (gp->totalFrames < ((u32*)entry)[2])
                        return 0;
                    return 1;
                }
                gp->sectorsRemaining = 0;
                gp->frameNumber = ((u32*)entry)[2];
                if (!(gp->totalFrames < ((u32*)entry)[2]))
                    break;
                gp->endOfStream = 1;
                return 0;
            }
            return 1;
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/I2Ddr
 */
s32 movie_get_next_audio_entry(void** outEntry)
{
    void** saved_arg;
    s32 nextIdx;
    u8* entry;
    u16 temp;

    /* Return immediately if nothing is available: ring empty and no secondary data pending */
    if ((MOVIE_STATE->audioWriteIdx == MOVIE_STATE->audioReadIdx) &&
        (MOVIE_STATE->lastAudioFrame == MOVIE_STATE->lastConsumedAudioFrame))
    {
        return 0;
    }

    /* Wrap readIdx back to 0 when it reaches the end of the ring */
    if ((((volatile MovieState*)0x801ED500)->audioWriteIdx <= MOVIE_STATE->audioReadIdx) &&
        (MOVIE_STATE->audioReadIdx == MOVIE_STATE->audioRingSize))
    {
        temp = MOVIE_STATE->audioWriteIdx != 0;
        MOVIE_STATE->audioReadIdx = 0;
        if (!temp && (MOVIE_STATE->lastAudioFrame == MOVIE_STATE->lastConsumedAudioFrame))
        {
            return 0;
        }
    }

    /* Look past already-buffered entries to find the next one to queue */
    nextIdx = MOVIE_STATE->audioReadIdx + MOVIE_STATE->audioBufferedCount;

    /* Wrap nextIdx if it overflows the ring */
    if ((MOVIE_STATE->audioReadIdx >= MOVIE_STATE->audioWriteIdx) &&
        (nextIdx >= ((volatile MovieState*)0x801ED500)->audioRingSize))
    {
        nextIdx -= MOVIE_STATE->audioRingSize;
    }

    /* All loaded entries are already queued; nothing new to dispatch */
    if ((nextIdx == MOVIE_STATE->audioWriteIdx) && (MOVIE_STATE->audioBufferedCount != 0))
    {
        return 0;
    }

    /* Resolve entry: each entry occupies one 2048-byte CD sector in the audio data buffer */
    entry = MOVIE_STATE->audioDataBase + (nextIdx << 11);
    temp = ((Entry*)entry)->sectorCount;
    MOVIE_STATE->audioBufferedCount += temp;
    *outEntry = entry;
    return 1;
}

/**
 * decomp.me: (100%) https://decomp.me/scratch/TApbR
 */
void movie_draw_sync_callback(void)
{
    volatile u8* base = (volatile u8*)0x801ED500;
    unsigned int temp_lo;
    int new_var;

    if (g_busy == 0)
    {
        base[0x97] = 0;

        {
            u8 tmp = base[0x9a];
            if (tmp != 0)
            {
                u8 idx = base[0x99];
                u32* ptr = (u32*)(base + (idx << 2));
                LoadImage((RECT*)(base + 0x30), (u_long*)ptr[6]);
                movie_schedule_next_decode();
                base[0x9a] = 0;
            }
        }

        {
            u8 tmp = base[0x9b];
            if (tmp != 0)
            {
                s16 v1 = *((s16*)(base + 0x34));
                s16 v2 = *((s16*)(base + 0x36));
                u8 idx = base[0x99];
                u32* ptr = (u32*)(base + (idx << 2));
                temp_lo = v1 * v2;
                new_var = temp_lo + (temp_lo >> 31);
                DecDCTout((u_long*)ptr[6], (s32)(new_var >> 1));
                base[0x9b] = 0;
            }
        }
    }
}

/**
 * decomp.me: (100%) https://decomp.me/scratch/OJvsJ
 */
s32 movie_get_next_video_entry(s32* outVlcData, s32* outEntryHeader)
{
    volatile MovieState* base = (volatile MovieState*)0x801ED500;
    MovieState* base2;
    s32 writeIdx;
    s32 readIdx;
    s32* out0 = outVlcData;
    s32* out1 = outEntryHeader;

    if (base->videoWriteIdx == base->videoReadIdx)
    {
        if (base->lastVideoFrame != base->lastConsumedVideoFrame)
        {
            base = MOVIE_STATE;
        }
        else
        {
            return 0;
        }
    }

    base = MOVIE_STATE;

    writeIdx = base->videoWriteIdx;
    readIdx = base->videoReadIdx;

    if ((readIdx >= writeIdx) && (readIdx == base->videoRingSize))
    {
        MOVIE_STATE->videoReadIdx = 0;
        if ((base->videoWriteIdx == 0) && (base->lastVideoFrame == base->lastConsumedVideoFrame))
        {
            return 0;
        }
    }

    base2 = MOVIE_STATE;
    *out1 = base2->videoTableBase + (base2->videoReadIdx << 5);
    *outVlcData = base2->videoDataBase + (base2->videoReadIdx * 2016);
    return 1;
}

/**
 * decomp.me: (100%) https://decomp.me/scratch/SUBK5
 */
void movie_advance_video_read(void)
{
    s32 nextIndex;
    SectorEntry* inner;
    MovieState* base = MOVIE_STATE;

    inner = (SectorEntry*)(base->videoTableBase + (base->videoReadIdx << 5));
    nextIndex = base->videoReadIdx + inner->sectorCount;

    if ((base->videoReadIdx >= base->videoWriteIdx) && (nextIndex == base->videoRingSize))
    {
        nextIndex = 0;
    }

    MOVIE_STATE->lastConsumedVideoFrame = inner->frameNumber;
    MOVIE_STATE->videoReadIdx = nextIndex;
}

/**
 * decomp.me: (100%) https://decomp.me/scratch/6Xjsu
 */
void movie_advance_audio_read(void)
{
    /* NOTE: comparing the audio nextIndex against videoRingSize (not
     * audioRingSize) below looks like an original-game bug; preserved
     * verbatim to keep the asm matching. */
    MovieState* base;
    SectorEntry* inner;
    s32 nextIndex;

    /* Base movie playback control block located at fixed RAM address (0x801ED500). */
    base = MOVIE_STATE;

    /* Resolve pointer to current audio sector header using read index (2048 bytes per sector). */
    inner = (SectorEntry*)(base->audioDataBase + (base->audioReadIdx << 11));

    /* Advance read index by number of sectors described in this header. */
    nextIndex = base->audioReadIdx + inner->sectorCount;

    /* Decrease buffered sector count to reflect consumed audio data. */
    base->audioBufferedCount = base->audioBufferedCount - inner->sectorCount;

    /* Wrap to start of ring buffer only if advancing from a "full" state hits capacity exactly. */
    if ((base->audioReadIdx >= base->audioWriteIdx) && (nextIndex == base->videoRingSize))
    {
        nextIndex = 0;
    }

    /* Update playback state: sync frame number and commit new read index. */
    MOVIE_STATE->lastConsumedAudioFrame = inner->frameNumber;
    MOVIE_STATE->audioReadIdx = nextIndex;
}