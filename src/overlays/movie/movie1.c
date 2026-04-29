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

        func_80140358((movieIndex & 0xFFFF) + 0x16A0, 0x80, frameCount, 0);
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
    func_801406E4();

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

    FUN_80140d48();
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
void func_80140358(s32 resourceIndex, s32 arg1, s32 totalFrames, int arg3)
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

    ((UnkState*)0x801ED500)->unk90 = (s8)(arg1 & 0x7F);
    if (arg1 & 0x80)
    {
        ((UnkState*)0x801ED500)->unk91 = 1;
    }
    else
    {
        ((UnkState*)0x801ED500)->unk91 = 0;
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
        ((UnkState*)0x801ED500)->unk50 = 0x32;
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
        ((UnkState*)0x801ED500)->unk54 = 0x10;
        ((UnkState*)0x801ED500)->unk4 = (u8*)0x80147640;
        ((UnkState*)0x801ED500)->unk98 = 0;
        ((UnkState*)new_var)->unkC = (u8*)p3;
    }
    else
    {
        p2 = 0x11000;
        p3 = arg3;
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
            new_var7->rects[2].x = (&new_var7->rects[arg3])->x;
            // FIX: cast to unsigned short to force zero-extension (lhu) instead of sign-extension (lh)
            p1 = (unsigned short)new_var7->rects[arg3].y;
        }
        new_var7->rects[2].w = 0x10;
        new_var7->unk50 = 0x1E;
        ((UnkState*)0x801ED500)->unk54 = 0x10;
        ((UnkState*)0x801ED500)->unk98 = (s8)arg3;
        ((UnkState*)0x801ED500)->unk4 = (u8*)(((u32)(*new_var8)) + 0x3C0);
        ((UnkState*)0x801ED500)->rects[2].y = p1;
    }

    ((UnkState*)0x801ED500)->unk44 = resourceIndex;
    ((UnkState*)0x801ED500)->unk48 = 0;
    ((UnkState*)0x801ED500)->unk4C = totalFrames;
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

    if (((UnkState*)0x801ED500)->unk91 != 0)
    {
        func_800232A8((u32)((UnkState*)0x801ED500)->unk8, (u32)(((UnkState*)0x801ED500)->unk54 << 0xB));
        func_80023030(0x7F);
    }
    else
    {
        func_80022848(0x7FFF);
        func_80022F18(0xA0);
    }

    cdrom_wait_queue_empty();
    new_var3 = (UnkState*)0x801ED500;
    cdrom_queue_command(CdlReadS, (s16)resourceIndex, NULL, &func_80140F04);

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
void func_801406E4(void)
{
    long pDispEnv;
    D_801ED500_t* new_var6;
    int new_var;
    void* sp10;
    void* sp14;
    D_801ED500_t* new_var2;
    s32 audioFadeVol = 0;
    D_801ED500_t* s0 = (D_801ED500_t*)0x801ED500;
    int new_var3;
    volatile int new_var4;
    if (D_801ED595 != 0)
    {
        new_var2 = s0;
        if ((new_var2->field9C == 0) && (s0->field9D == 0))
        {
            s0->field9C = 1;
            DecDCTin((u_long*)s0->ptr10[s0->field93], (s0->field90 & 0xFFFFu) == 0);
            {
                s32 temp = ((s16)s0->field34) * ((s16)s0->field36);
                new_var3 = temp + (((unsigned)temp) >> 31);
                DecDCTout((u_long*)s0->ptr18[s0->field99], new_var3 >> 1);
            }
            s0->field95 = 0;
        }
    }
    if ((D_801ED595 == 0) & 0xFFFFu)
    {
        ;
        {
            u8 v0 = ((D_801ED500_t*)0x801ED500)->field94;
            if (v0 != 0)
            {
                v0--;
                ((D_801ED500_t*)0x801ED500)->field94 = v0;
                if ((v0 & 0xFF) == 0)
                {
                    DecDCTvlcSize2(0);
                }
                if (DecDCTvlc2(0, 0, (DECDCTTAB*)((D_801ED500_t*)0x801ED500)->table) == 0)
                {
                    audioFadeVol = 1;
                    ((D_801ED500_t*)0x801ED500)->field94 = 0;
                }
            }
            else if (movie_get_next_video_entry(&sp10, &sp14) != 0)
            {
                ((D_801ED500_t*)0x801ED500)->field48 = ((u32*)sp14)[2];
                new_var6 = (D_801ED500_t*)0x801ED500;
                if ((((u32*)sp14)[2] >= ((D_801ED500_t*)0x801ED500)->field4C) && (new_var6->field9F == 0))
                {
                    ((D_801ED500_t*)0x801ED500)->field9F = 1;
                }
                {
                    int one;
                    ((D_801ED500_t*)0x801ED500)->field93 = 1 - ((D_801ED500_t*)0x801ED500)->field93;
                }
                if (((D_801ED500_t*)0x801ED500)->field90 == 0)
                {
                    DecDCTvlcSize2(0x1000);
                    ((D_801ED500_t*)0x801ED500)->field94 = 3;
                }
                else
                {
                    DecDCTvlcSize2(0x16AA);
                    ((D_801ED500_t*)0x801ED500)->field94 = 1;
                }
                if (DecDCTvlc2((u_long*)sp10,
                               (u_long*)((D_801ED500_t*)0x801ED500)->ptr10[((D_801ED500_t*)0x801ED500)->field93],
                               (DECDCTTAB*)((D_801ED500_t*)0x801ED500)->table) == 0)
                {
                    audioFadeVol = 1;
                    ((D_801ED500_t*)0x801ED500)->field94 = 0;
                }
            }
            else
            {
                if (((!sp14) && (!sp14)) && (!sp14))
                {
                }
                if ((((D_801ED500_t*)0x801ED500)->field9E != 0) && (((D_801ED500_t*)0x801ED500)->field9C == 0))
                {
                    ((D_801ED500_t*)0x801ED500)->field9F = 2;
                }
            }
        }
    }
    new_var3 = 0;
    if (audioFadeVol != new_var3)
    {
        movie_advance_video_read();
        s0 = (D_801ED500_t*)0x801ED500;
        if ((s0->field9C == new_var3) && (new_var = s0->field9D == new_var3))
        {
            s0->field9C = 1;
            DecDCTin((u_long*)s0->ptr10[s0->field93], s0->field90 == 0);
            {
                s32 temp = ((s16)s0->field34) * ((s16)s0->field36);
                new_var = ((unsigned)temp) >> 31;
                DecDCTout((u_long*)s0->ptr18[s0->field99], (temp + new_var) >> 1);
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
        if (func_8014159C(&sp10) != 0)
        {
            new_var4 = (s0->field48 = ((u32*)sp10)[2]);
            if ((new_var4 > s0->field4C) && (s0->field9F < 2))
            {
                s0->field9F = 2;
            }
            func_80023334(new_var4);
        }
        s0 = (D_801ED500_t*)0x801ED500;
        if (D_801ED592 == 2)
        {
            s0 = (D_801ED500_t*)0x801ED500;
            pDispEnv = s0->field54;
            audioFadeVol = s0->field70;
            if (audioFadeVol >= ((s32)(pDispEnv >> 1)))
            {
                func_8002246C(3);
                s0->field92 = 0;
            }
        }
        s0 = (D_801ED500_t*)0x801ED500;
        if ((s0->field64 != ((D_801ED500_t*)0x801ED500)->field68) ||
            (((D_801ED500_t*)0x801ED500)->field88 != ((D_801ED500_t*)0x801ED500)->field8C))
        {
            s32 tmp = func_800233B8();
            if (((tmp != (-1)) && (((D_801ED500_t*)0x801ED500)->field70 != 0)) &&
                (((D_801ED500_t*)0x801ED500)->field68 != ((u32)(tmp * 2))))
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
        func_80140C00();
        return;
    }
    bp->unk9C = 1;
}

/**
 * decomp.me: (98.72%) https://decomp.me/scratch/E7XCZ
 */
void func_80140C00(void)
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
    nextOutBufIdx = 1 - (*((volatile u8*)(&ptr->unk99)));
    curFramePos = *((volatile u16*)(&ptr->unk30));
    frameStep = *((volatile u16*)(&ptr->unk34));
    newFramePos = curFramePos + frameStep;
    *((volatile u16*)(&ptr->unk30)) = newFramePos;
    newFramePosSigned = (s16)newFramePos;
    *((volatile u8*)(&ptr->unk99)) = nextOutBufIdx;
    a = ptr->ch[*((volatile u8*)(&ptr->unk98))].a;
    c = ptr->ch[*((volatile u8*)(&ptr->unk98))].c;
    chunkEnd = a + c;
    if (newFramePosSigned < chunkEnd)
    {
        if ((*((volatile u8*)(&ptr->unk97))) < 2U)
        {
            decodeSize = ((s16)frameStep) * ((s16)(*((volatile u16*)(&ptr->unk36))));
            decodeWordCount = ((int)(decodeSize + (decodeSize >> 31))) >> 1;
            DecDCTout((u32*)ptr->unk18[*((volatile u8*)(&ptr->unk99))], decodeWordCount);
            *((volatile u8*)(&ptr->unk9C)) = 2;
        }
        else
        {
            *((volatile u8*)(&ptr->unk9C)) = 1;
            *((volatile u8*)(&ptr->unk9B)) = 1;
        }
    }
    else
    {
        // determining which of the GFX buffer to use from the double buffer?
        *((volatile u8*)(&ptr->unk98)) = 1 - (*((volatile u8*)(&ptr->unk98)));
        ptr->unk30 = ptr->ch[*((volatile u8*)(&ptr->unk98))].a;
        ptr->unk32 = *(new_var = &ptr->ch[*((volatile u8*)(&ptr->unk98))].b);
        *((volatile u8*)(&ptr->unk9D)) = 1;
        *((volatile u8*)(&ptr->unk9C)) = 0;
        if ((*((volatile u8*)(&ptr->unk9F))) == 1)
        {
            *((volatile u8*)(&ptr->unk9F)) = 2;
        }
    }
}

/**
 * decomp.me (93.87%) https://decomp.me/scratch/JTTFr
 */
void FUN_80140d48(void)
{
    volatile GlobalStruct* G = (volatile GlobalStruct*)0x801ED500;
    int new_var2;
    u_long* new_var;
    if (!G->unk9A)
    {
        if (!G->unk9B)
        {
            return;
        }
    }
    if (G->unk90 == 0)
    {
        if (DrawSync(1) >= 2)
        {
            return;
        }
        if (G->unk9A)
        {
            u8 t = G->unk9A;
            if (t)
            {
                G->unk96 = 1;
                t = G->unk99;
                LoadImage((RECT*)0x801ED530, (u_long*)G->ptrArray[t]);
                G->unk97 = DrawSync(1) + 1;
                G->unk9A = 0;
                func_80140C00();
            }
            G->unk96 = 0;
        }
        G = (volatile GlobalStruct*)0x801ED500;
        if (G->unk9B)
        {
            u8 t = G->unk9B;
            if (t)
            {
                s32 temp;
                G->unk96 = 1;
                temp = ((s32)G->unk34) * ((s32)G->unk36);
                new_var2 = temp + (((u32)temp) >> 31);
                DecDCTout((u_long*)G->ptrArray[G->unk99], new_var2 >> 1);
                G->unk9B = 0;
            }
            G->unk96 = 0;
        }
    }
    else // <-- changed block starts here
    {
        if (G->unk9A)
        {
            u8 t = G->unk9A;
            if (t)
            {
                s32 bd;
                G->unk96 = 1;
                new_var = BreakDraw();
                bd = (s32)new_var;
                if (bd != (-1))
                {
                    LoadImage2((RECT*)0x801ED530, (u_long*)G->ptrArray[G->unk99]);
                    if (bd != 0)
                    {
                        DrawOTag((u_long*)bd);
                    }
                    func_80140C00();
                    G->unk9A = 0;
                }
            }
        }
        D_801ED596 = 0; // <-- moved outside the inner if
    }
}

/**
 * decomp.me (76.45%) https://decomp.me/scratch/HptYe
 */
s32 func_80140F04(void)
{
    SectorBuffer sp10;
    s32 do_load; /* s0 in assembly */
    volatile GlobalData* const gp = (GlobalData*)0x801ED500;
    u16* sp16;
    u32 count;
    u16 rem;
    void* dest;
    void* entry;

    do_load = 0;

    if (D_801ED57E == 0)
    {
        /* read first header sector */
        while (CdGetSector(sp10, 8) == 0)
        {
        }

        if (gp->totalFrames < sp10[2])
        {
            gp->unk9E = 1;
            return 0;
        }

        gp->frameNumber = sp10[2];

        /* check low word of sp10[1] (offset 0x14) */
        if (((u16*)sp10)[2] != 0)
            return 1;

        /* check high word of sp10[0] (offset 0x12) */
        if (((u16*)sp10)[1] == 0x8001)
        {
            /* 0x8001 sector type */
            if (gp->unk58 == gp->unk5C)
            {
                if (gp->unk80 == gp->unk84)
                {
                    count = ((u16*)sp10)[3];
                    if (gp->unk50 < gp->unk58 + (s32)count)
                    {
                        if (gp->unk5C >= (s32)count)
                        {
                            do_load = 1;
                            gp->unk60 = gp->unk58;
                            gp->unk58 = 0;
                        }
                    }
                    else
                    {
                        do_load = 1;
                    }
                }
            }
            else if (gp->unk5C < gp->unk58)
            {
                count = ((u16*)sp10)[3];
                if (gp->unk50 < gp->unk58 + (s32)count)
                {
                    if (gp->unk5C >= (s32)count)
                    {
                        do_load = 1;
                        gp->unk60 = gp->unk58;
                        gp->unk58 = 0;
                    }
                }
                else
                {
                    do_load = 1;
                }
            }
            else
            {
                count = ((u16*)sp10)[3];
                if (gp->unk5C >= gp->unk58 + (s32)count)
                {
                    do_load = 1;
                }
            }

            if (do_load != 0)
            {
                dest = (void*)(gp->unk4 + (gp->unk58 * 2016));
                while (CdGetSector(dest, 0x1F8) == 0)
                {
                }

                entry = (void*)(gp->unk0 + (gp->unk58 << 5));
                ((u32*)entry)[0] = sp10[0];
                ((u32*)entry)[1] = sp10[1];
                ((u32*)entry)[2] = sp10[2];
                ((u32*)entry)[3] = sp10[3];
                ((u32*)entry)[4] = sp10[4];
                ((u32*)entry)[5] = sp10[5];
                ((u32*)entry)[6] = sp10[6];
                ((u32*)entry)[7] = sp10[7];

                rem = (u16)(((u16*)sp10)[3] - 1);
                gp->unk7E = rem;
                if (rem == 0)
                {
                    gp->unk58 += 1;
                    gp->unk80 = gp->frameNumber;
                    return (gp->frameNumber < gp->totalFrames) ? 1 : 0;
                }
                else
                {
                    gp->unk78 = 0;
                    gp->unk7C = rem;
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
            if (gp->unk64 == gp->unk68)
            {
                if (gp->unk88 == gp->unk8C)
                {
                    count = ((u16*)sp10)[3];
                    if (gp->unk54 < gp->unk64 + (s32)count)
                    {
                        if (gp->unk68 >= (s32)count)
                        {
                            do_load = 1;
                            gp->unk6C = gp->unk64;
                            gp->unk64 = 0;
                        }
                    }
                    else
                    {
                        do_load = 1;
                    }
                }
            }
            else if (gp->unk68 < gp->unk64)
            {
                count = ((u16*)sp10)[3];
                if (gp->unk54 < gp->unk64 + (s32)count)
                {
                    if (gp->unk68 >= (s32)count)
                    {
                        do_load = 1;
                        gp->unk6C = gp->unk64;
                        gp->unk64 = 0;
                    }
                }
                else
                {
                    do_load = 1;
                }
            }
            else
            {
                count = ((u16*)sp10)[3];
                if (gp->unk68 >= gp->unk64 + (s32)count)
                {
                    do_load = 1;
                }
            }

            if (do_load != 0)
            {
                dest = (void*)(gp->unk8 + (gp->unk64 << 11) + 0x20);
                while (CdGetSector(dest, 0x1F8) == 0)
                {
                }

                entry = (void*)(gp->unk8 + (gp->unk64 << 11));
                ((u32*)entry)[0] = sp10[0];
                ((u32*)entry)[1] = sp10[1];
                ((u32*)entry)[2] = sp10[2];
                ((u32*)entry)[3] = sp10[3];
                ((u32*)entry)[4] = sp10[4];
                ((u32*)entry)[5] = sp10[5];
                ((u32*)entry)[6] = sp10[6];
                ((u32*)entry)[7] = sp10[7];

                rem = (u16)(((u16*)sp10)[3] - 1);
                gp->unk7E = rem;
                if (rem == 0)
                {
                    gp->unk64 += 1;
                    gp->unk88 = gp->frameNumber;
                    if (gp->totalFrames < gp->frameNumber)
                        return 0;
                }
                else
                {
                    gp->unk78 = rem;
                    gp->unk7C = rem;
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
        if (gp->unk78 == 0)
        {
            for (;;)
            {
                entry = (void*)(gp->unk0 + ((gp->unk58 + gp->unk7C) << 5));
                while (CdGetSector(entry, 8) == 0)
                {
                }

                sp16 = (u16*)entry;
                if (sp16[1] == 0x8001 && ((u32*)entry)[2] == gp->frameNumber && sp16[2] == gp->unk7C)
                {
                    dest = (void*)(gp->unk4 + ((gp->unk58 + gp->unk7C) * 2016));
                    while (CdGetSector(dest, 0x1F8) == 0)
                    {
                    }

                    rem = gp->unk7E - 1;
                    gp->unk7E = rem;
                    if (rem != 0)
                    {
                        gp->unk7C += 1;
                        return 1;
                    }
                    gp->unk58 = gp->unk58 + 1 + gp->unk7C;
                    gp->unk80 = gp->frameNumber;
                    return (((u32*)entry)[2] < gp->totalFrames) ? 1 : 0;
                }
                gp->unk7E = 0;
                gp->frameNumber = ((u32*)entry)[2];
                if (((u32*)entry)[2] < gp->totalFrames)
                    break;
                gp->unk9E = 1;
                return 0;
            }
            return 1;
        }
        else
        {
            for (;;)
            {
                entry = (void*)(gp->unk8 + ((gp->unk64 + gp->unk7C) << 11));
                while (CdGetSector(entry, 8) == 0)
                {
                }

                sp16 = (u16*)entry;
                if (sp16[1] == 1 && ((u32*)entry)[2] == gp->frameNumber && sp16[2] == gp->unk7C)
                {
                    dest = (void*)(gp->unk8 + ((gp->unk64 + gp->unk7C) << 11) + 0x20);
                    while (CdGetSector(dest, 0x1F8) == 0)
                    {
                    }

                    rem = gp->unk7E - 1;
                    gp->unk7E = rem;
                    if (rem != 0)
                    {
                        gp->unk7C += 1;
                        return 1;
                    }
                    gp->unk64 = gp->unk64 + 1 + gp->unk7C;
                    gp->unk88 = gp->frameNumber;
                    if (gp->totalFrames < ((u32*)entry)[2])
                        return 0;
                    return 1;
                }
                gp->unk7E = 0;
                gp->frameNumber = ((u32*)entry)[2];
                if (!(gp->totalFrames < ((u32*)entry)[2]))
                    break;
                gp->unk9E = 1;
                return 0;
            }
            return 1;
        }
    }
}

/**
 * decomp.me (82.91%) https://decomp.me/scratch/I2Ddr
 */
s32 func_8014159C(void** arg0)
{
    void** saved_arg;
    s32 var;
    u8* entry_ptr;
    volatile Global* new_var2;
    u16 temp;

    if (((volatile Global*)0x801ED500)->unk64 != ((volatile Global*)0x801ED500)->unk68)
    {
    }
    else if (((volatile Global*)0x801ED500)->unk88 != ((volatile Global*)0x801ED500)->unk8C)
    {
    }
    else
    {
        return 0;
    }

    if (((volatile Global*)0x801ED500)->unk64 <= ((volatile Global*)0x801ED500)->unk68)
    {
        if (((volatile Global*)0x801ED500)->unk68 == ((volatile Global*)0x801ED500)->unk6C)
        {
            temp = ((volatile Global*)0x801ED500)->unk64 != 0;
            ((volatile Global*)0x801ED500)->unk68 = 0;
            if (temp || (((volatile Global*)0x801ED500)->unk88 != ((volatile Global*)0x801ED500)->unk8C))
            {
            }
            else
            {
                return 0;
            }
        }
    }

    var = ((volatile Global*)0x801ED500)->unk68 + ((volatile Global*)0x801ED500)->unk70;

    if (((volatile Global*)0x801ED500)->unk68 >= ((volatile Global*)0x801ED500)->unk64)
    {
        if (var >= ((volatile Global*)0x801ED500)->unk6C)
        {
            var -= ((volatile Global*)0x801ED500)->unk6C;
        }
    }
    new_var2 = (volatile Global*)0x801ED500;
    if ((var == ((volatile Global*)0x801ED500)->unk64) && (new_var2->unk70 != 0))
    {
        return 0;
    }
    entry_ptr = ((volatile Global*)0x801ED500)->unk8 + (var << 11);
    temp = ((Entry*)entry_ptr)->sectorCount;
    ((volatile Global*)0x801ED500)->unk70 += temp;
    *arg0 = entry_ptr;
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
                func_80140C00();
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