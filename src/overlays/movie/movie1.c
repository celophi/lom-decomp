#include "movie.h"

/**
 * decomp.me link (97.51%) https://decomp.me/scratch/XvMvo
 * this one is a WIP without gotos (https://decomp.me/scratch/Gq1vj)
 */
void movie_play(s32 movieIndex)
{
    DISPENV env[2];
    DISPENV* pDispEnv;
    volatile SRC_801ED500* p500;
    s32 audioFadeVol;
    s32 new_var;
    s32 new_var3;
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
    new_var3 = 5;
    p500 = (SRC_801ED500*)0x801ED500;
    new_var = 2;
    goto error_loop;

error_loop_retry:
    if (error_status == new_var3)
    {
        goto recheck_unk9d;
    }

    func_800157B0(1);
    new_var3++;
    new_var3--;
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
        u8 unk9d_val = p500->frameReady;
        if (unk9d_val != 0)
        {
            goto after_wait;
        }
    }

    if (p500->unk9f == new_var)
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
    if (p500->frameReady == 0)
    {
        goto wait_loop;
    }

    p500->frameReady = 0;
    func_800157B0(4);
    VSync(0);

    pDispEnv = &env[0];
    if (p500->activeDisplayBuffer == 0)
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

    if (p500->unk9f != new_var)
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
 * decomp.me link (91.61%) https://decomp.me/scratch/tw6Km
 * incorrect but better match https://decomp.me/scratch/ICOiP
 */
void movie_init(s32 resourceIndex, s32 flags, s32 totalFrames, s32 initBufferIdx)
{
    u32 p1;
    u8* new_var6;
    UnkState* new_var3;
    UnkState* new_var2;
    u32 p2;
    UnkState* new_var4;
    u32 p3;
    UnkState* new_var7;
    u8* new_var9;
    u8** new_var8;
    int new_var;
    u8** pDispEnv;
    AllocInfo* allocInfo = D_80180014;

    ((UnkState*)0x801ED500)->gpuMode = (s8)(flags & 0x7F);
    if (flags & 0x80)
    {
        ((UnkState*)0x801ED500)->interlaceMode = 1;
    }
    else
    {
        ((UnkState*)0x801ED500)->interlaceMode = 0;
    }

    pDispEnv = &((UnkState*)0x801ED500)->unk0;

    if (D_801ED590 == 0)
    {
        p3 = 0x80168000;
        p2 = 0x80179000;
        new_var6 = (u8*)0x80147000;
        p1 = 0x8018D000;

        ((UnkState*)0x801ED500)->unk0 = new_var6;
        p2++;
        p2--;
        ((UnkState*)0x801ED500)->unk1C = (u8*)0x801A3D00;
        ((UnkState*)0x801ED500)->unk8 = (u8*)0x80160000;
        ((UnkState*)0x801ED500)->unk84 = (u32)(-1);
        ((UnkState*)0x801ED500)->rects[1].w = 0x1E0;
        (new_var4 = (UnkState*)0x801ED500)->rects[0].w = 0x1E0;
        ((UnkState*)0x801ED500)->rects[2].w = 0x18;
        ((UnkState*)0x801ED500)->videoRingCapacity = 0x32;
        new_var = 0x801ED500;
        ((UnkState*)0x801ED500)->unk10 = (u8*)p2;
        ((UnkState*)0x801ED500)->unk14 = (u8*)p1;
        ((UnkState*)0x801ED500)->unk18 = (u8*)0x801A1000;
        ((UnkState*)0x801ED500)->rects[0].x = p3 * 0;
        ((UnkState*)0x801ED500)->rects[1].x = 0;
        ((UnkState*)0x801ED500)->rects[0].y = 0;
        ((UnkState*)0x801ED500)->rects[1].y = 0xF0;
        ((UnkState*)0x801ED500)->rects[2].h = 0xF0;
        ((UnkState*)0x801ED500)->rects[1].h = 0xF0;
        ((UnkState*)0x801ED500)->rects[0].h = 0xF0;
        ((UnkState*)0x801ED500)->rects[2].x = 0;
        ((UnkState*)0x801ED500)->rects[2].y = 0;
        ((UnkState*)0x801ED500)->audioRingCapacity = 0x10;
        ((UnkState*)0x801ED500)->unk4 = (u8*)0x80147640;
        ((UnkState*)0x801ED500)->chunkIdx = 0;
        ((UnkState*)new_var)->unkC = (u8*)p3;
    }
    else
    {
        p2 = 0x11000;
        p3 = initBufferIdx;
        ((UnkState*)0x801ED500)->unk0 = (u8*)0x80147000;
        ((UnkState*)0x801ED500)->unk8 = (u8*)0x80156000;
        new_var6 = (u8*)allocInfo->unk38;
        ((UnkState*)0x801ED500)->unk10 = (u8*)0x8015E000;
        ((UnkState*)0x801ED500)->unk14 = (u8*)0x8016F000;
        new_var8 = &(*pDispEnv);
        new_var9 = new_var6;
        ((UnkState*)0x801ED500)->unkC = new_var9;
        ((UnkState*)0x801ED500)->unk18 = (u8*)(allocInfo->unk38 + p2);
        ((UnkState*)0x801ED500)->unk1C = (u8*)(allocInfo->unk38 + 0x12E00);

        if (((s16)((UnkState*)0x801ED500)->rects[0].x) >= 0x300)
        {
            ((UnkState*)0x801ED500)->rects[1].x = 0x200;
            ((UnkState*)0x801ED500)->rects[1].y = 0;
        }
        else
        {
            ((UnkState*)0x801ED500)->rects[1].x =
                (u16)(((UnkState*)0x801ED500)->rects[0].x + ((UnkState*)0x801ED500)->rects[0].w);
            ((UnkState*)0x801ED500)->rects[1].y = ((UnkState*)0x801ED500)->rects[0].y;
        }

        new_var7 = (UnkState*)0x801ED500;
        new_var7->rects[1].w = new_var7->rects[0].w;
        {
            new_var7->rects[1].h = new_var7->rects[0].h;
            new_var7->rects[2].h = new_var7->rects[0].h;
            new_var7->rects[2].x = (&new_var7->rects[initBufferIdx])->x;
            // FIX: cast to unsigned short to force zero-extension (lhu) instead of sign-extension (lh)
            p1 = (unsigned short)new_var7->rects[initBufferIdx].y;
        }
        new_var7->rects[2].w = 0x10;
        new_var7->videoRingCapacity = 0x1E;
        ((UnkState*)0x801ED500)->audioRingCapacity = 0x10;
        ((UnkState*)0x801ED500)->chunkIdx = (s8)initBufferIdx;
        ((UnkState*)0x801ED500)->unk4 = (u8*)(((u32)(*new_var8)) + 0x3C0);
        ((UnkState*)0x801ED500)->rects[2].y = p1;
    }

    ((UnkState*)0x801ED500)->resourceIndex = resourceIndex;
    ((UnkState*)0x801ED500)->currentFrame = 0;
    ((UnkState*)0x801ED500)->totalFrames = totalFrames;
    ((UnkState*)0x801ED500)->unk93 = 0;
    ((UnkState*)0x801ED500)->unk94 = 0;
    ((UnkState*)0x801ED500)->unk95 = 0;
    ((UnkState*)0x801ED500)->unk96 = 0;
    ((UnkState*)0x801ED500)->unk97 = 0;
    ((UnkState*)0x801ED500)->unk99 = 0;
    ((UnkState*)0x801ED500)->unk9A = 0;
    ((UnkState*)0x801ED500)->unk9B = 0;
    ((UnkState*)0x801ED500)->unk9C = 0;
    ((UnkState*)0x801ED500)->unk9D = 0;
    ((UnkState*)0x801ED500)->unk9E = 0;
    ((UnkState*)0x801ED500)->unk9F = 0;
    ((UnkState*)0x801ED500)->unk92 = 0;

    ((UnkState*)0x801ED500)->unk58[0] = 0;
    ((UnkState*)0x801ED500)->unk58[1] = 0;
    ((UnkState*)0x801ED500)->unk58[2] = 0;
    ((UnkState*)0x801ED500)->unk58[3] = 0;
    ((UnkState*)0x801ED500)->unk58[4] = 0;
    ((UnkState*)0x801ED500)->unk58[5] = 0;
    ((UnkState*)0x801ED500)->unk58[6] = 0;
    ((UnkState*)0x801ED500)->unk58[7] = 0;
    ((UnkState*)0x801ED500)->unk58[8] = 0;

    ((UnkState*)0x801ED500)->unk7E = 0;
    ((UnkState*)0x801ED500)->unk80 = (u32)(-1);
    ((UnkState*)0x801ED500)->unk84 = (u32)(-1);
    ((UnkState*)0x801ED500)->unk88 = (u32)(-1);
    ((UnkState*)0x801ED500)->unk8C = (u32)(-1);

    ((UnkState*)0x801ED500)->unk38 = (u32)DecDCToutCallback(&movie_mdec_out_callback, p1, p2, p3);
    ((UnkState*)0x801ED500)->unk3C = DrawSyncCallback(&movie_draw_sync_callback);

    if (((UnkState*)0x801ED500)->interlaceMode != 0)
    {
        func_800232A8((u32)((UnkState*)0x801ED500)->unk8, (u32)(((UnkState*)0x801ED500)->audioRingCapacity << 0xB));
        func_80023030(0x7F);
    }
    else
    {
        func_80022848(0x7FFF);
        func_80022F18(0xA0);
    }

    cdrom_wait_queue_empty();
    new_var3 = (UnkState*)0x801ED500;
    cdrom_queue_command(CdlReadS, (s16)resourceIndex, NULL, &movie_cd_sector_callback);

    if (D_801ED590 == 0)
    {
        VSync(p2 = 0);
        SetDispMask(p2);
        ClearImage(&(new_var2 = new_var3)->rects[p2], 0, p2, 0);
        ClearImage(&((UnkState*)0x801ED500)->rects[1], p2, 0, p2);
        DecDCTvlcBuild((u_short*)new_var3->unkC);
        DrawSync(0);
    }
}

/**
 * decomp.me link (98.86%) https://decomp.me/scratch/bjwdC
 */
void movie_update(void)
{
    long pDispEnv;
    D_801ED500_t* new_var6;
    int new_var;
    void* hdr;
    void* sp14;
    D_801ED500_t* new_var2;
    s32 audioFadeVol = 0;
    D_801ED500_t* s0 = (D_801ED500_t*)0x801ED500;
    int new_var3;
    volatile int new_var4;
    if (D_801ED595 != 0)
    {
        new_var2 = s0;
        if ((new_var2->mdecBusy == 0) && (s0->field9D == 0))
        {
            s0->mdecBusy = 1;
            DecDCTin((u_long*)s0->ptr10[s0->inputBufIdx], (s0->gpuMode & 0xFFFFu) == 0);
            {
                s32 temp = ((s16)s0->field34) * ((s16)s0->field36);
                new_var3 = temp + (((unsigned)temp) >> 31);
                DecDCTout((u_long*)s0->ptr18[s0->outBufIdx], new_var3 >> 1);
            }
            s0->field95 = 0;
        }
    }
    if ((D_801ED595 == 0) & 0xFFFFu)
    {
        ;
        {
            u8 v0 = ((D_801ED500_t*)0x801ED500)->vlcRetryCount;
            if (v0 != 0)
            {
                v0--;
                ((D_801ED500_t*)0x801ED500)->vlcRetryCount = v0;
                if ((v0 & 0xFF) == 0)
                {
                    DecDCTvlcSize2(0);
                }
                if (DecDCTvlc2(0, 0, (DECDCTTAB*)((D_801ED500_t*)0x801ED500)->table) == 0)
                {
                    audioFadeVol = 1;
                    ((D_801ED500_t*)0x801ED500)->vlcRetryCount = 0;
                }
            }
            else if (movie_get_next_video_entry(&hdr, &sp14) != 0)
            {
                ((D_801ED500_t*)0x801ED500)->currentFrame = ((u32*)sp14)[2];
                new_var6 = (D_801ED500_t*)0x801ED500;
                if ((((u32*)sp14)[2] >= ((D_801ED500_t*)0x801ED500)->totalFrames) && (new_var6->endState == 0))
                {
                    ((D_801ED500_t*)0x801ED500)->endState = 1;
                }
                {
                    int one;
                    ((D_801ED500_t*)0x801ED500)->inputBufIdx = 1 - ((D_801ED500_t*)0x801ED500)->inputBufIdx;
                }
                if (((D_801ED500_t*)0x801ED500)->gpuMode == 0)
                {
                    DecDCTvlcSize2(0x1000);
                    ((D_801ED500_t*)0x801ED500)->vlcRetryCount = 3;
                }
                else
                {
                    DecDCTvlcSize2(0x16AA);
                    ((D_801ED500_t*)0x801ED500)->vlcRetryCount = 1;
                }
                if (DecDCTvlc2((u_long*)hdr,
                               (u_long*)((D_801ED500_t*)0x801ED500)->ptr10[((D_801ED500_t*)0x801ED500)->inputBufIdx],
                               (DECDCTTAB*)((D_801ED500_t*)0x801ED500)->table) == 0)
                {
                    audioFadeVol = 1;
                    ((D_801ED500_t*)0x801ED500)->vlcRetryCount = 0;
                }
            }
            else
            {
                if (((!sp14) && (!sp14)) && (!sp14))
                {
                }
                if ((((D_801ED500_t*)0x801ED500)->endOfStream != 0) && (((D_801ED500_t*)0x801ED500)->mdecBusy == 0))
                {
                    ((D_801ED500_t*)0x801ED500)->endState = 2;
                }
            }
        }
    }
    new_var3 = 0;
    if (audioFadeVol != new_var3)
    {
        movie_advance_video_read();
        s0 = (D_801ED500_t*)0x801ED500;
        if ((s0->mdecBusy == new_var3) && (new_var = s0->field9D == new_var3))
        {
            s0->mdecBusy = 1;
            DecDCTin((u_long*)s0->ptr10[s0->inputBufIdx], s0->gpuMode == 0);
            {
                s32 temp = ((s16)s0->field34) * ((s16)s0->field36);
                new_var = ((unsigned)temp) >> 31;
                DecDCTout((u_long*)s0->ptr18[s0->outBufIdx], (temp + new_var) >> 1);
            }
        }
        else
        {
            D_801ED595 = 1;
        }
    }
    s0 = (D_801ED500_t*)0x801ED500;
    if (g_cdAudioReady != 0)
    {
        if (movie_get_next_audio_entry(&hdr) != 0)
        {
            new_var4 = (s0->currentFrame = ((u32*)hdr)[2]);
            if ((new_var4 > s0->totalFrames) && (s0->endState < 2))
            {
                s0->endState = 2;
            }
            func_80023334(new_var4);
        }
        s0 = (D_801ED500_t*)0x801ED500;
        if (D_801ED592 == 2)
        {
            s0 = (D_801ED500_t*)0x801ED500;
            pDispEnv = s0->audioRingCapacity;
            audioFadeVol = s0->audioBufferedCount;
            if (audioFadeVol >= ((s32)(pDispEnv >> 1)))
            {
                func_8002246C(3);
                s0->field92 = 0;
            }
        }
        s0 = (D_801ED500_t*)0x801ED500;
        if ((s0->audioWriteIdx != ((D_801ED500_t*)0x801ED500)->audioReadIdx) ||
            (((D_801ED500_t*)0x801ED500)->lastAudioFrame != ((D_801ED500_t*)0x801ED500)->unk8C))
        {
            s32 tmp = func_800233B8();
            if (((tmp != (-1)) && (((D_801ED500_t*)0x801ED500)->audioBufferedCount != 0)) &&
                (((D_801ED500_t*)0x801ED500)->audioReadIdx != ((u32)(tmp * 2))))
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
    if (D_801ED590 == 0)
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
        D_801ED596 = 0; // <-- moved outside the inner if
    }
}

/**
 * decomp.me (76.45%) https://decomp.me/scratch/HptYe
 *
 * CD sector-arrival callback, called by the CD-ROM interrupt once per sector read.
 *
 * Reads the 8-word (32-byte) sector header, determines if the sector is video
 * (type 0x8001) or audio, checks whether the corresponding ring buffer has room,
 * then copies the 504-word (2016-byte) payload into the buffer and updates the
 * write index.  Multi-sector frames are handled via D_801ED57E: when 0 this is
 * the first (header) sector; when non-zero we are reading continuation sectors
 * for the same frame and skip the ring-capacity check.
 *
 * Returns 1 to keep streaming, 0 when the stream has ended or should pause.
 */
s32 movie_cd_sector_callback(void)
{
    SectorBuffer hdr;       /* 32-byte sector header (8 u32 words) read from CD */
    s32 do_load; /* s0 in assembly */
    volatile GlobalData* const gp = (GlobalData*)0x801ED500;
    u16* hdr16;  /* u16 view of hdr for field access by word index */
    u32 count;
    u16 rem;
    void* dest;
    void* entry;

    do_load = 0;

    if (D_801ED57E == 0)
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
                            gp->videoWrapSavedIdx = gp->videoWriteIdx;
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
                        gp->videoWrapSavedIdx = gp->videoWriteIdx;
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
                            gp->audioWrapSavedIdx = gp->audioWriteIdx;
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
                        gp->audioWrapSavedIdx = gp->audioWriteIdx;
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

            if (D_801ED592 == 1)
            {
                gp->unk92 = 2;
            }
            return 1;
        }
    }
    else
    {
        /* D_801ED57E != 0 */
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
    if ((((Global*)0x801ED500)->audioWriteIdx == ((Global*)0x801ED500)->audioReadIdx) &&
        (((Global*)0x801ED500)->unk88 == ((Global*)0x801ED500)->unk8C))
    {
        return 0;
    }

    /* Wrap readIdx back to 0 when it reaches the end of the ring */
    if ((((volatile Global*)0x801ED500)->audioWriteIdx <= ((Global*)0x801ED500)->audioReadIdx) &&
        (((Global*)0x801ED500)->audioReadIdx == ((Global*)0x801ED500)->audioRingSize))
    {
        temp = ((Global*)0x801ED500)->audioWriteIdx != 0;
        ((Global*)0x801ED500)->audioReadIdx = 0;
        if (!temp && (((Global*)0x801ED500)->unk88 == ((Global*)0x801ED500)->unk8C))
        {
            return 0;
        }
    }

    /* Look past already-buffered entries to find the next one to queue */
    nextIdx = ((Global*)0x801ED500)->audioReadIdx + ((Global*)0x801ED500)->audioBufferedCount;

    /* Wrap nextIdx if it overflows the ring */
    if ((((Global*)0x801ED500)->audioReadIdx >= ((Global*)0x801ED500)->audioWriteIdx) &&
        (nextIdx >= ((volatile Global*)0x801ED500)->audioRingSize))
    {
        nextIdx -= ((Global*)0x801ED500)->audioRingSize;
    }

    /* All loaded entries are already queued; nothing new to dispatch */
    if ((nextIdx == ((Global*)0x801ED500)->audioWriteIdx) && (((Global*)0x801ED500)->audioBufferedCount != 0))
    {
        return 0;
    }

    /* Resolve entry: each entry occupies one 2048-byte CD sector in the audio data buffer */
    entry = ((Global*)0x801ED500)->audioDataBase + (nextIdx << 11);
    temp = ((Entry*)entry)->sectorCount;
    ((Global*)0x801ED500)->audioBufferedCount += temp;
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

    if (D_801ED596 == 0)
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
    volatile BaseStruct_80141788* base = (volatile BaseStruct_80141788*)0x801ED500;
    BaseStruct_80141788* base2;
    s32 writeIdx;
    s32 readIdx;
    s32* out0 = outVlcData;
    s32* out1 = outEntryHeader;

    if (base->videoWriteIdx == base->videoReadIdx)
    {
        if (base->unk80 != base->lastConsumedVideoFrame)
        {
            base = (BaseStruct_80141788*)0x801ED500;
        }
        else
        {
            return 0;
        }
    }

    base = (BaseStruct_80141788*)0x801ED500;

    writeIdx = base->videoWriteIdx;
    readIdx = base->videoReadIdx;

    if ((readIdx >= writeIdx) && (readIdx == base->ringCapacity))
    {
        ((BaseStruct_80141788*)0x801ED500)->videoReadIdx = 0;
        if ((base->videoWriteIdx == 0) && (base->unk80 == base->lastConsumedVideoFrame))
        {
            return 0;
        }
    }

    base2 = (BaseStruct_80141788*)0x801ED500;
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
    BaseStruct_80141788* base = (BaseStruct_80141788*)0x801ED500;

    inner = (InnerStruct*)(base->videoTableBase + (base->videoReadIdx << 5));
    nextIndex = base->videoReadIdx + inner->sectorCount;

    if ((base->videoReadIdx >= base->videoWriteIdx) && (nextIndex == base->ringCapacity))
    {
        nextIndex = 0;
    }

    ((BaseStruct_80141788*)0x801ED500)->lastConsumedVideoFrame = inner->frameNumber;
    ((BaseStruct_80141788*)0x801ED500)->videoReadIdx = nextIndex;
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
    if ((base->audioReadIdx >= base->audioWriteIdx) && (nextIndex == base->ringCapacity))
    {
        nextIndex = 0;
    }

    /* Update playback state: sync frame number and commit new read index. */
    ((BaseStruct_801418B0*)0x801ED500)->frameNumber = inner->frameNumber;
    ((BaseStruct_801418B0*)0x801ED500)->audioReadIdx = nextIndex;
}