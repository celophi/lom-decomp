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
 * decomp.me link (97.51%) https://decomp.me/scratch/XvMvo
 * this one is a WIP without gotos (https://decomp.me/scratch/Gq1vj)
 */
void movie_play(s32 movieIndex)
{
    DISPENV env[2];
    DISPENV* pDispEnv;
    volatile SRC_801ED500* state;
    s32 audioFadeVol;
    s32 endStateMatch;     /* state->unk9f comparison constant (always 2) */
    s32 retryLimit;        /* cdrom error-loop retry threshold (always 5) */
    s32 error_status;
    s32 timeout;

    VSync(0);
    func_800157DC();
    func_800157B0(1);
    VSync(0);
    func_800157DC();
    cdrom_process_state();

    if ((movieIndex & 0xFFFF) == 0)
    {
        if (((SRC_801ED600*)0x801ED600)->unk0 < 3)
        {
            if ((((SRC_801ED600*)0x801ED600)->unk2 & 0xFF0F) != 0)
            {
                return;
            }
        }
    }

    func_800158E0();
    DecDCTReset(0);
    timeout = 0xF0;

    SetDefDispEnv(&env[0], 0, 0, 320, timeout);
    SetDefDispEnv(&env[1], 0, timeout, 320, timeout);
    env[0].isrgb24 = (env[1].isrgb24 = 1);

    {
        u32 frameCount;
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
    }

    VSync(0);
    func_800157DC();
    audioFadeVol = -1;
    retryLimit = 5;
    state = (SRC_801ED500*)0x801ED500;
    endStateMatch = 2;
    goto error_loop;

error_loop_retry:
    if (error_status == retryLimit)
    {
        goto recheck_unk9d;
    }

    func_800157B0(1);
    retryLimit++;
    retryLimit--;
    VSync(0);
    func_800157DC();
    cdrom_process_state();

error_loop:
    error_status = cdrom_get_error_status();

    if (error_status != 0)
    {
        goto error_loop_retry;
    }
    goto recheck_unk9d;

wait_loop:
    movie_update();

    {
        u8 unk9d_val = state->frameReady;
        if (unk9d_val != 0)
        {
            goto after_wait;
        }
    }

    if (state->unk9f == endStateMatch)
    {
        goto cleanup;
    }

    movie_service_video_ops();
    if ((--timeout) != 0)
    {
        goto wait_loop;
    }

after_wait:
    if (timeout != 0)
    {
        goto recheck_unk9d;
    }

    cdrom_process_state();

recheck_unk9d:
    timeout = 0x2000;
    if (state->frameReady == 0)
    {
        goto wait_loop;
    }

    state->frameReady = 0;
    func_800157B0(4);
    VSync(0);

    pDispEnv = &env[0];
    if (state->activeDisplayBuffer == 0)
    {
        pDispEnv = &env[1];
    }

    PutDispEnv(pDispEnv);
    SetDispMask(1);
    func_800157DC();
    cdrom_process_state();

    {
        u32 a0 = (u32)(movieIndex & 0xFFFF);
        if ((a0 < 2) && (((SRC_801ED600*)0x801ED600)->unk0 < 3))
        {
            u16 val = ((SRC_801ED600*)0x801ED600)->unk4;
            if (a0 != 0)
            {
                if ((val & 0x400A) != 0)
                {
                    goto set_audioFadeVol;
                }
                goto check_audio_call;
            }
            if ((val & 0xFF0F) != 0)
            {
                goto set_audioFadeVol;
            }
            goto check_audio_call;

            timeout = 0xF0;

        set_audioFadeVol:
            if (g_cdAudioReady == 0)
            {
                goto cleanup;
            }

            if (audioFadeVol == (-1))
            {
                audioFadeVol = 0x70;
            }
        }
    }

check_audio_call:
    if ((g_cdAudioReady != 0) && (audioFadeVol != (-1)))
    {
        func_80023030(audioFadeVol);
        if (audioFadeVol == 0)
        {
            goto cleanup;
        }
        audioFadeVol -= 0x10;
    }

    if (state->unk9f != endStateMatch)
    {
        goto error_loop;
    }

cleanup:
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
    u8* vlcTablePtr;             /* (was new_var6) base of VLC code table */
    CombinedState* stateRef2;    /* (was new_var3) used after VSync; alias of 0x801ED500 */
    CombinedState* stateRef3;    /* (was new_var2) used as ClearImage arg base */
    u32 p2;
    CombinedState* stateRef4;    /* (was new_var4) sequencing alias from rect setup */
    u32 p3;
    CombinedState* stateRef5;    /* (was stateRef5) sequencing alias from rect copies */
    u8* vlcTablePtr2;            /* (was new_var9) duplicate of vlcTablePtr (forces an extra move) */
    u8** videoTableBasePtr;      /* (was new_var8) &state->videoTableBase reload */
    int stateAddrInt;            /* (was new_var) literal 0x801ED500 used for vlcTable store */
    u8** videoTableBaseRef;      /* (was pDispEnv) NOT a DISPENV; it's &state->videoTableBase */
    AllocInfo* allocInfo = g_allocInfo;

    ((CombinedState*)0x801ED500)->gpuMode = (s8)(flags & 0x7F);
    if (flags & 0x80)
    {
        ((CombinedState*)0x801ED500)->interlaceMode = 1;
    }
    else
    {
        ((CombinedState*)0x801ED500)->interlaceMode = 0;
    }

    videoTableBaseRef = &((CombinedState*)0x801ED500)->videoTableBase;

    if (g_gpuMode == 0)
    {
        p3 = 0x80168000;
        p2 = 0x80179000;
        vlcTablePtr = (u8*)0x80147000;
        p1 = 0x8018D000;

        ((CombinedState*)0x801ED500)->videoTableBase = vlcTablePtr;
        p2++;
        p2--;
        ((CombinedState*)0x801ED500)->mdecOutputBuf[1] = (u8*)0x801A3D00;
        ((CombinedState*)0x801ED500)->audioDataBase = (u8*)0x80160000;
        ((CombinedState*)0x801ED500)->lastConsumedVideoFrame = (u32)(-1);
        ((CombinedState*)0x801ED500)->rects[1].w = 0x1E0;
        (stateRef4 = (CombinedState*)0x801ED500)->rects[0].w = 0x1E0;
        ((CombinedState*)0x801ED500)->rects[2].w = 0x18;
        ((CombinedState*)0x801ED500)->videoRingCapacity = 0x32;
        stateAddrInt = 0x801ED500;
        ((CombinedState*)0x801ED500)->vlcInputBuf[0] = (u8*)p2;
        ((CombinedState*)0x801ED500)->vlcInputBuf[1] = (u8*)p1;
        ((CombinedState*)0x801ED500)->mdecOutputBuf[0] = (u8*)0x801A1000;
        ((CombinedState*)0x801ED500)->rects[0].x = p3 * 0;
        ((CombinedState*)0x801ED500)->rects[1].x = 0;
        ((CombinedState*)0x801ED500)->rects[0].y = 0;
        ((CombinedState*)0x801ED500)->rects[1].y = 0xF0;
        ((CombinedState*)0x801ED500)->rects[2].h = 0xF0;
        ((CombinedState*)0x801ED500)->rects[1].h = 0xF0;
        ((CombinedState*)0x801ED500)->rects[0].h = 0xF0;
        ((CombinedState*)0x801ED500)->rects[2].x = 0;
        ((CombinedState*)0x801ED500)->rects[2].y = 0;
        ((CombinedState*)0x801ED500)->audioRingCapacity = 0x10;
        ((CombinedState*)0x801ED500)->videoDataBase = (u8*)0x80147640;
        ((CombinedState*)0x801ED500)->chunkIdx = 0;
        ((CombinedState*)stateAddrInt)->vlcTable = p3;
    }
    else
    {
        p2 = 0x11000;
        p3 = initBufferIdx;
        ((CombinedState*)0x801ED500)->videoTableBase = (u8*)0x80147000;
        ((CombinedState*)0x801ED500)->audioDataBase = (u8*)0x80156000;
        vlcTablePtr = (u8*)allocInfo->allocBase;
        ((CombinedState*)0x801ED500)->vlcInputBuf[0] = (u8*)0x8015E000;
        ((CombinedState*)0x801ED500)->vlcInputBuf[1] = (u8*)0x8016F000;
        videoTableBasePtr = &(*videoTableBaseRef);
        vlcTablePtr2 = vlcTablePtr;
        ((CombinedState*)0x801ED500)->vlcTable = vlcTablePtr2;
        ((CombinedState*)0x801ED500)->mdecOutputBuf[0] = (u8*)(allocInfo->allocBase + p2);
        ((CombinedState*)0x801ED500)->mdecOutputBuf[1] = (u8*)(allocInfo->allocBase + 0x12E00);

        if (((s16)((CombinedState*)0x801ED500)->rects[0].x) >= 0x300)
        {
            ((CombinedState*)0x801ED500)->rects[1].x = 0x200;
            ((CombinedState*)0x801ED500)->rects[1].y = 0;
        }
        else
        {
            ((CombinedState*)0x801ED500)->rects[1].x =
                (u16)(((CombinedState*)0x801ED500)->rects[0].x + ((CombinedState*)0x801ED500)->rects[0].w);
            ((CombinedState*)0x801ED500)->rects[1].y = ((CombinedState*)0x801ED500)->rects[0].y;
        }

        stateRef5 = (CombinedState*)0x801ED500;
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
        ((CombinedState*)0x801ED500)->audioRingCapacity = 0x10;
        ((CombinedState*)0x801ED500)->chunkIdx = (s8)initBufferIdx;
        ((CombinedState*)0x801ED500)->videoDataBase = (u8*)(((u32)(*videoTableBasePtr)) + 0x3C0);
        ((CombinedState*)0x801ED500)->rects[2].y = p1;
    }

    ((CombinedState*)0x801ED500)->resourceIndex = resourceIndex;
    ((CombinedState*)0x801ED500)->currentFrame = 0;
    ((CombinedState*)0x801ED500)->totalFrames = totalFrames;
    ((CombinedState*)0x801ED500)->inputBufIdx = 0;
    ((CombinedState*)0x801ED500)->vlcRetryCount = 0;
    ((CombinedState*)0x801ED500)->mdecRetryPending = 0;
    ((CombinedState*)0x801ED500)->busy = 0;
    ((CombinedState*)0x801ED500)->unk97 = 0;
    ((CombinedState*)0x801ED500)->outBufIdx = 0;
    ((CombinedState*)0x801ED500)->unk9A = 0;
    ((CombinedState*)0x801ED500)->unk9B = 0;
    ((CombinedState*)0x801ED500)->mdecBusy = 0;
    ((CombinedState*)0x801ED500)->field9D = 0;
    ((CombinedState*)0x801ED500)->endOfStream = 0;
    ((CombinedState*)0x801ED500)->endState = 0;
    ((CombinedState*)0x801ED500)->field92 = 0;

    ((CombinedState*)0x801ED500)->ringPadding0[0] = 0;
    ((CombinedState*)0x801ED500)->ringPadding0[1] = 0;
    ((CombinedState*)0x801ED500)->ringPadding0[2] = 0;
    ((CombinedState*)0x801ED500)->audioWriteIdx = 0;
    ((CombinedState*)0x801ED500)->audioReadIdx = 0;
    ((CombinedState*)0x801ED500)->ringPadding1 = 0;
    ((CombinedState*)0x801ED500)->audioBufferedCount = 0;
    ((CombinedState*)0x801ED500)->ringPadding2[0] = 0;
    ((CombinedState*)0x801ED500)->ringPadding2[1] = 0;

    ((CombinedState*)0x801ED500)->unk7E = 0;
    ((CombinedState*)0x801ED500)->unk80 = (u32)(-1);
    ((CombinedState*)0x801ED500)->lastConsumedVideoFrame = (u32)(-1);
    ((CombinedState*)0x801ED500)->lastAudioFrame = (u32)(-1);
    ((CombinedState*)0x801ED500)->lastConsumedAudioFrame = (u32)(-1);

    ((CombinedState*)0x801ED500)->decDCToutCallback = (u32)DecDCToutCallback(&movie_mdec_out_callback, p1, p2, p3);
    ((CombinedState*)0x801ED500)->drawSyncCallback = DrawSyncCallback(&movie_draw_sync_callback);

    if (((CombinedState*)0x801ED500)->interlaceMode != 0)
    {
        func_800232A8((u32)((CombinedState*)0x801ED500)->audioDataBase,
                      (u32)(((CombinedState*)0x801ED500)->audioRingCapacity << 0xB));
        func_80023030(0x7F);
    }
    else
    {
        func_80022848(0x7FFF);
        func_80022F18(0xA0);
    }

    cdrom_wait_queue_empty();
    stateRef2 = (CombinedState*)0x801ED500;
    cdrom_queue_command(CdlReadS, (s16)resourceIndex, NULL, &movie_cd_sector_callback);

    if (g_gpuMode == 0)
    {
        VSync(p2 = 0);
        SetDispMask(p2);
        ClearImage(&(stateRef3 = stateRef2)->rects[p2], 0, p2, 0);
        ClearImage(&((CombinedState*)0x801ED500)->rects[1], p2, 0, p2);
        DecDCTvlcBuild((u_short*)stateRef2->vlcTable);
        DrawSync(0);
    }
}

/**
 * decomp.me link (98.86%) https://decomp.me/scratch/xXKIt
 */
void movie_update(void)
{
    long audioCapacity;             /* (was pDispEnv) audioRingCapacity reload */
    CombinedState* stateAlias;      /* (was new_var6) sequencing alias used after totalFrames check */
    int field9DZeroFlag;            /* (was new_var) (field9D == 0) flag / sign-shift temp */
    void* hdr;
    void* sp14;
    CombinedState* mdecAlias;       /* (was new_var2) sequencing alias used in MDEC retry block */
    s32 tmp = 0;                    /* (was audioFadeVol) reused: vlc-complete flag, then audioBufferedCount compare */
    CombinedState* combined = (CombinedState*)0x801ED500;
    int wordCount;                  /* (was new_var3) DCT word count temp / zero literal */
    volatile int audioFrameNum;     /* (was new_var4) frame number from latest audio entry */
    if (g_mdecRetryPending != 0)
    {
        mdecAlias = combined;
        if ((mdecAlias->mdecBusy == 0) && (combined->field9D == 0))
        {
            combined->mdecBusy = 1;
            DecDCTin((u_long*)((CombinedState*)combined)->vlcInputBuf[combined->inputBufIdx],
                     (combined->gpuMode & 0xFFFFu) == 0);
            {
                s32 temp =
                    ((s16)((CombinedState*)combined)->rects[2].w) * ((s16)((CombinedState*)combined)->rects[2].h);
                wordCount = temp + (((unsigned)temp) >> 31);
                DecDCTout((u_long*)(((CombinedState*)combined)->mdecOutputBuf[combined->outBufIdx]), wordCount >> 1);
            }
            combined->mdecRetryPending = 0;
        }
    }
    if ((g_mdecRetryPending == 0) & 0xFFFFu)
    {
        ;
        {
            u8 v0 = ((CombinedState*)0x801ED500)->vlcRetryCount;
            if (v0 != 0)
            {
                v0--;
                ((CombinedState*)0x801ED500)->vlcRetryCount = v0;
                if ((v0 & 0xFF) == 0)
                {
                    DecDCTvlcSize2(0);
                }
                if (DecDCTvlc2(0, 0, (DECDCTTAB*)((CombinedState*)0x801ED500)->vlcTable) == 0)
                {
                    tmp = 1;
                    ((CombinedState*)0x801ED500)->vlcRetryCount = 0;
                }
            }
            else if (movie_get_next_video_entry(&hdr, &sp14) != 0)
            {
                ((CombinedState*)0x801ED500)->currentFrame = ((u32*)sp14)[2];
                stateAlias = (CombinedState*)0x801ED500;
                if ((((u32*)sp14)[2] >= ((CombinedState*)0x801ED500)->totalFrames) && (stateAlias->endState == 0))
                {
                    ((CombinedState*)0x801ED500)->endState = 1;
                }
                {
                    int one;
                    ((CombinedState*)0x801ED500)->inputBufIdx = 1 - ((CombinedState*)0x801ED500)->inputBufIdx;
                }
                if (((CombinedState*)0x801ED500)->gpuMode == 0)
                {
                    DecDCTvlcSize2(0x1000);
                    ((CombinedState*)0x801ED500)->vlcRetryCount = 3;
                }
                else
                {
                    DecDCTvlcSize2(0x16AA);
                    ((CombinedState*)0x801ED500)->vlcRetryCount = 1;
                }
                if (DecDCTvlc2(
                        (u_long*)hdr,
                        (u_long*)((CombinedState*)0x801ED500)->vlcInputBuf[((CombinedState*)0x801ED500)->inputBufIdx],
                        (DECDCTTAB*)((CombinedState*)0x801ED500)->vlcTable) == 0)
                {
                    tmp = 1;
                    ((CombinedState*)0x801ED500)->vlcRetryCount = 0;
                }
            }
            else
            {
                if (((!sp14) && (!sp14)) && (!sp14))
                {
                }
                if ((((CombinedState*)0x801ED500)->endOfStream != 0) && (((CombinedState*)0x801ED500)->mdecBusy == 0))
                {
                    ((CombinedState*)0x801ED500)->endState = 2;
                }
            }
        }
    }
    wordCount = 0;
    if (tmp != wordCount)
    {
        movie_advance_video_read();
        combined = (CombinedState*)0x801ED500;
        if ((combined->mdecBusy == wordCount) && (field9DZeroFlag = combined->field9D == wordCount))
        {
            combined->mdecBusy = 1;
            DecDCTin((u_long*)((CombinedState*)combined)->vlcInputBuf[combined->inputBufIdx], combined->gpuMode == 0);
            {
                s32 temp =
                    ((s16)((CombinedState*)combined)->rects[2].w) * ((s16)((CombinedState*)combined)->rects[2].h);
                field9DZeroFlag = ((unsigned)temp) >> 31;
                DecDCTout((u_long*)(((CombinedState*)combined)->mdecOutputBuf[combined->outBufIdx]),
                          (temp + field9DZeroFlag) >> 1);
            }
        }
        else
        {
            g_mdecRetryPending = 1;
        }
    }
    combined = (CombinedState*)0x801ED500;
    if (g_cdAudioReady != 0)
    {
        if (movie_get_next_audio_entry(&hdr) != 0)
        {
            audioFrameNum = (combined->currentFrame = ((u32*)hdr)[2]);
            if ((audioFrameNum > combined->totalFrames) && (combined->endState < 2))
            {
                combined->endState = 2;
            }
            func_80023334(audioFrameNum);
        }
        combined = (CombinedState*)0x801ED500;
        if (g_audioStreamState == 2)
        {
            combined = (CombinedState*)0x801ED500;
            audioCapacity = combined->audioRingCapacity;
            tmp = combined->audioBufferedCount;
            if (tmp >= ((s32)(audioCapacity >> 1)))
            {
                func_8002246C(3);
                combined->field92 = 0;
            }
        }
        combined = (CombinedState*)0x801ED500;
        if ((combined->audioWriteIdx != ((CombinedState*)0x801ED500)->audioReadIdx) ||
            (((CombinedState*)0x801ED500)->lastAudioFrame != ((CombinedState*)0x801ED500)->lastConsumedAudioFrame))
        {
            s32 tmp = func_800233B8();
            if (((tmp != (-1)) && (((CombinedState*)0x801ED500)->audioBufferedCount != 0)) &&
                (((CombinedState*)0x801ED500)->audioReadIdx != ((u32)(tmp * 2))))
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
    volatile BaseObj* base = (volatile BaseObj*)0x801ED500;
    s32 temp;
    BaseObj* bp_high;
    BaseObj* bp;
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
            LoadImage((RECT*)0x801ED530, ((SubObj*)(((u_char*)base) + (((u_long)((u8)base->unk99)) * 4)))->unk18);
            base->unk97 = (s8)(temp + 1);
        }
        else
        {
            base->unk9A = 1U;
        }
        bp_high = (BaseObj*)0x801e0000;
    }
    else
    {
        temp = (s32)BreakDraw();
        new_var = 0;
        if (temp != (-1))
        {
            LoadImage2((RECT*)0x801ED530, ((SubObj*)(((u_char*)base) + (((u_long)((u8)base->unk99)) * 4)))->unk18);
            if (temp != new_var)
            {
                DrawOTag((u_long*)temp);
            }
            bp_high = (BaseObj*)0x801e0000;
        }
        else
        {
            LoadImage((RECT*)0x801ED530, ((SubObj*)(((u_char*)base) + (((u_long)((u8)base->unk99)) * 4)))->unk18);
        }
    }

    bp = (BaseObj*)(((u_int)((BaseObj*)0x801e0000)) | 0xd500);
    if (bp->unk9A == new_var)
    {
        movie_schedule_next_decode();
        return;
    }
    bp->unk9C = 1;
}

/**
 * decomp.me: (98.72%) https://decomp.me/scratch/E7XCZ
 */
void movie_schedule_next_decode(void)
{
    Struct_801ED500* ptr = (Struct_801ED500*)0x801ED500;
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
    curFramePos = *((volatile u16*)(&ptr->framePos));
    frameStep = *((volatile u16*)(&ptr->unk34));
    newFramePos = curFramePos + frameStep;
    *((volatile u16*)(&ptr->framePos)) = newFramePos;
    newFramePosSigned = (s16)newFramePos;
    *((volatile u8*)(&ptr->outBufIdx)) = nextOutBufIdx;
    a = ptr->ch[*((volatile u8*)(&ptr->chunkIdx))].start;
    c = ptr->ch[*((volatile u8*)(&ptr->chunkIdx))].length;
    chunkEnd = a + c;
    if (newFramePosSigned < chunkEnd)
    {
        if ((*((volatile u8*)(&ptr->unk97))) < 2U)
        {
            decodeSize = ((s16)frameStep) * ((s16)(*((volatile u16*)(&ptr->unk36))));
            decodeWordCount = ((int)(decodeSize + (decodeSize >> 31))) >> 1;
            DecDCTout((u32*)ptr->unk18[*((volatile u8*)(&ptr->outBufIdx))], decodeWordCount);
            *((volatile u8*)(&ptr->decodeState)) = 2;
        }
        else
        {
            *((volatile u8*)(&ptr->decodeState)) = 1;
            *((volatile u8*)(&ptr->pendingMdecDecode)) = 1;
        }
    }
    else
    {
        /* advance to the next chunk and reset the frame position */
        *((volatile u8*)(&ptr->chunkIdx)) = 1 - (*((volatile u8*)(&ptr->chunkIdx)));
        ptr->framePos = ptr->ch[*((volatile u8*)(&ptr->chunkIdx))].start;
        ptr->unk32 = *(new_var = &ptr->ch[*((volatile u8*)(&ptr->chunkIdx))].b);
        *((volatile u8*)(&ptr->frameReady)) = 1;
        *((volatile u8*)(&ptr->decodeState)) = 0;
        if ((*((volatile u8*)(&ptr->endState))) == 1)
        {
            *((volatile u8*)(&ptr->endState)) = 2;
        }
    }
}

/**
 * decomp.me (93.87%) https://decomp.me/scratch/JTTFr
 */
/*
 * Service pending video output operations.
 *
 * Two flags gate the two halves:
 *   pendingVramUpload — a decoded frame is ready; DMA it into VRAM (LoadImage) and
 *                       kick off the MDEC decode of the next frame (movie_schedule_next_decode).
 *   pendingMdecDecode — new BS bitstream data is staged; feed it to the MDEC (DecDCTout).
 *
 * gpuMode selects the transfer path:
 *   0         — wait for DrawSync then use LoadImage (standard DMA)
 *   non-zero  — interrupt the current draw via BreakDraw then use LoadImage2
 */
void movie_service_video_ops(void)
{
    volatile GlobalStruct* G = (volatile GlobalStruct*)0x801ED500;
    int wordCount;
    u_long* breakDrawResult;
    if (!G->pendingVramUpload)
    {
        if (!G->pendingMdecDecode)
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
        if (G->pendingVramUpload)
        {
            u8 t = G->pendingVramUpload;
            if (t)
            {
                G->busy = 1;
                t = G->activeBufferIdx;
                LoadImage((RECT*)0x801ED530, (u_long*)G->ptrArray[t]);
                G->drawSyncTarget = DrawSync(1) + 1;
                G->pendingVramUpload = 0;
                movie_schedule_next_decode();
            }
            G->busy = 0;
        }
        G = (volatile GlobalStruct*)0x801ED500;
        if (G->pendingMdecDecode)
        {
            u8 t = G->pendingMdecDecode;
            if (t)
            {
                s32 temp;
                G->busy = 1;
                /* word count = (width * height) / 2, rounded toward zero for signed values */
                temp = ((s32)G->unk34) * ((s32)G->unk36);
                wordCount = temp + (((u32)temp) >> 31);
                DecDCTout((u_long*)G->ptrArray[G->activeBufferIdx], wordCount >> 1);
                G->pendingMdecDecode = 0;
            }
            G->busy = 0;
        }
    }
    else // <-- changed block starts here
    {
        /* BreakDraw path: interrupt the current draw primitive list to upload immediately */
        if (G->pendingVramUpload)
        {
            u8 t = G->pendingVramUpload;
            if (t)
            {
                s32 bd;
                G->busy = 1;
                breakDrawResult = BreakDraw();
                bd = (s32)breakDrawResult;
                if (bd != (-1))
                {
                    LoadImage2((RECT*)0x801ED530, (u_long*)G->ptrArray[G->activeBufferIdx]);
                    if (bd != 0)
                    {
                        /* Resume the interrupted OTag list */
                        DrawOTag((u_long*)bd);
                    }
                    movie_schedule_next_decode();
                    G->pendingVramUpload = 0;
                }
            }
        }
        g_busy = 0; // <-- moved outside the inner if
    }
}

/**
 * decomp.me (76.45%) https://decomp.me/scratch/HptYe
 *
 * BUG NOTE: `gp->audioDataBase4` and `gp->audioDataBaseC` (below) are not
 * fields of GlobalData. From the surrounding ring-empty checks they are
 * almost certainly meant to be `lastConsumedVideoFrame` and
 * `lastConsumedAudioFrame` respectively. Fixing them changes the load
 * offset (0x0C vs 0x84/0x8C) which alters the asm, so leave as-is until
 * the canonical struct layout is confirmed.
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
    volatile GlobalData* const gp = (GlobalData*)0x801ED500;
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
                if (gp->lastVideoFrame == gp->audioDataBase4)
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
                if (gp->lastAudioFrame == gp->audioDataBaseC)
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
    if ((((GlobalData*)0x801ED500)->audioWriteIdx == ((GlobalData*)0x801ED500)->audioReadIdx) &&
        (((GlobalData*)0x801ED500)->lastAudioFrame == ((GlobalData*)0x801ED500)->lastConsumedAudioFrame))
    {
        return 0;
    }

    /* Wrap readIdx back to 0 when it reaches the end of the ring */
    if ((((volatile GlobalData*)0x801ED500)->audioWriteIdx <= ((GlobalData*)0x801ED500)->audioReadIdx) &&
        (((GlobalData*)0x801ED500)->audioReadIdx == ((GlobalData*)0x801ED500)->audioRingSize))
    {
        temp = ((GlobalData*)0x801ED500)->audioWriteIdx != 0;
        ((GlobalData*)0x801ED500)->audioReadIdx = 0;
        if (!temp && (((GlobalData*)0x801ED500)->lastAudioFrame == ((GlobalData*)0x801ED500)->lastConsumedAudioFrame))
        {
            return 0;
        }
    }

    /* Look past already-buffered entries to find the next one to queue */
    nextIdx = ((GlobalData*)0x801ED500)->audioReadIdx + ((GlobalData*)0x801ED500)->audioBufferedCount;

    /* Wrap nextIdx if it overflows the ring */
    if ((((GlobalData*)0x801ED500)->audioReadIdx >= ((GlobalData*)0x801ED500)->audioWriteIdx) &&
        (nextIdx >= ((volatile GlobalData*)0x801ED500)->audioRingSize))
    {
        nextIdx -= ((GlobalData*)0x801ED500)->audioRingSize;
    }

    /* All loaded entries are already queued; nothing new to dispatch */
    if ((nextIdx == ((GlobalData*)0x801ED500)->audioWriteIdx) && (((GlobalData*)0x801ED500)->audioBufferedCount != 0))
    {
        return 0;
    }

    /* Resolve entry: each entry occupies one 2048-byte CD sector in the audio data buffer */
    entry = ((GlobalData*)0x801ED500)->audioDataBase + (nextIdx << 11);
    temp = ((Entry*)entry)->sectorCount;
    ((GlobalData*)0x801ED500)->audioBufferedCount += temp;
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
    volatile GlobalData* base = (volatile GlobalData*)0x801ED500;
    GlobalData* base2;
    s32 writeIdx;
    s32 readIdx;
    s32* out0 = outVlcData;
    s32* out1 = outEntryHeader;

    if (base->videoWriteIdx == base->videoReadIdx)
    {
        if (base->lastVideoFrame != base->lastConsumedVideoFrame)
        {
            base = (GlobalData*)0x801ED500;
        }
        else
        {
            return 0;
        }
    }

    base = (GlobalData*)0x801ED500;

    writeIdx = base->videoWriteIdx;
    readIdx = base->videoReadIdx;

    if ((readIdx >= writeIdx) && (readIdx == base->videoRingSize))
    {
        ((GlobalData*)0x801ED500)->videoReadIdx = 0;
        if ((base->videoWriteIdx == 0) && (base->lastVideoFrame == base->lastConsumedVideoFrame))
        {
            return 0;
        }
    }

    base2 = (GlobalData*)0x801ED500;
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
    InnerStruct* inner;
    GlobalData* base = (GlobalData*)0x801ED500;

    inner = (InnerStruct*)(base->videoTableBase + (base->videoReadIdx << 5));
    nextIndex = base->videoReadIdx + inner->sectorCount;

    if ((base->videoReadIdx >= base->videoWriteIdx) && (nextIndex == base->videoRingSize))
    {
        nextIndex = 0;
    }

    ((GlobalData*)0x801ED500)->lastConsumedVideoFrame = inner->frameNumber;
    ((GlobalData*)0x801ED500)->videoReadIdx = nextIndex;
}

/**
 * decomp.me: (100%) https://decomp.me/scratch/6Xjsu
 */
void movie_advance_audio_read(void)
{
    BaseStruct_801418B0* base;
    InnerStruct_801418B0* inner;
    s32 nextIndex;

    /* Base movie playback control block located at fixed RAM address (0x801ED500). */
    base = (BaseStruct_801418B0*)0x801ED500;

    /* Resolve pointer to current audio sector header using read index (2048 bytes per sector). */
    inner = (InnerStruct_801418B0*)(base->audioDataBase + (base->audioReadIdx << 11));

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
    ((BaseStruct_801418B0*)0x801ED500)->lastConsumedAudioFrame = inner->frameNumber;
    ((BaseStruct_801418B0*)0x801ED500)->audioReadIdx = nextIndex;
}