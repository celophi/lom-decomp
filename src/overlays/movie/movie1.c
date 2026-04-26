#include "movie.h"

/**
 * decomp.me link (97.51%) https://decomp.me/scratch/XvMvo
 * this one is a WIP without gotos (https://decomp.me/scratch/Gq1vj)
 */
void FUN_80140018(s32 arg0)
{
    DISPENV env[2];
    DISPENV* new_var5;
    volatile SRC_801ED500* p500;
    s32 var_s2;
    s32 new_var;
    s32 new_var3;
    s32 error_status;
    s32 timeout;

    VSync(0);
    func_800157DC();
    func_800157B0(1);
    VSync(0);
    func_800157DC();
    CD_UpdateAndProcessQueue();

    if ((arg0 & 0xFFFF) == 0)
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
        u32 a2;
        switch ((u16)(arg0 & 0xFFFF))
        {
        case 0:
            a2 = 0x832;
            break;

        case 1:
            a2 = 0x9a9;
            break;

        case 2:
            a2 = 0x526;
            break;

        case 3:
            a2 = 0x14f8;
            break;

        case 4:

        default:
            a2 = 0x382;
            break;
        }

        func_80140358((arg0 & 0xFFFF) + 0x16A0, 0x80, a2, 0);
    }

    VSync(0);
    func_800157DC();
    var_s2 = -1;
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
    CD_UpdateAndProcessQueue();

error_loop:
    error_status = CD_GetErrorStatus();

    if (error_status != 0)
    {
        goto error_loop_retry;
    }
    goto recheck_unk9d;

wait_loop:
    func_801406E4();

    {
        u8 unk9d_val = p500->unk9d;
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

    CD_UpdateAndProcessQueue();

recheck_unk9d:
    timeout = 0x2000;
    if (p500->unk9d == 0)
    {
        goto wait_loop;
    }

    p500->unk9d = 0;
    func_800157B0(4);
    VSync(0);

    new_var5 = &env[0];
    if (p500->unk98 == 0)
    {
        new_var5 = &env[1];
    }

    PutDispEnv(new_var5);
    SetDispMask(1);
    func_800157DC();
    CD_UpdateAndProcessQueue();

    {
        u32 a0 = (u32)(arg0 & 0xFFFF);
        if ((a0 < 2) && (((SRC_801ED600*)0x801ED600)->unk0 < 3))
        {
            u16 val = ((SRC_801ED600*)0x801ED600)->unk4;
            if (a0 != 0)
            {
                if ((val & 0x400A) != 0)
                {
                    goto set_var_s2;
                }
                goto check_audio_call;
            }
            if ((val & 0xFF0F) != 0)
            {
                goto set_var_s2;
            }
            goto check_audio_call;

            timeout = 0xF0;

        set_var_s2:
            if (g_cdAudioReady == 0)
            {
                goto cleanup;
            }

            if (var_s2 == (-1))
            {
                var_s2 = 0x70;
            }
        }
    }

check_audio_call:
    if ((g_cdAudioReady != 0) && (var_s2 != (-1)))
    {
        func_80023030(var_s2);
        if (var_s2 == 0)
        {
            goto cleanup;
        }
        var_s2 -= 0x10;
    }

    if (p500->unk9f != new_var)
    {
        goto error_loop;
    }

cleanup:
    func_800158E0();

    CD_ResetSystem();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}

/**
 * decomp.me link (91.61%) https://decomp.me/scratch/tw6Km
 * incorrect but better match https://decomp.me/scratch/ICOiP
 */
void func_80140358(s32 arg0, s32 arg1, s32 arg2, int arg3)
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
    u8** new_var5;
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

    new_var5 = &((UnkState*)0x801ED500)->unk0;

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
        new_var8 = &(*new_var5);
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

    ((UnkState*)0x801ED500)->unk44 = arg0;
    ((UnkState*)0x801ED500)->unk48 = 0;
    ((UnkState*)0x801ED500)->unk4C = arg2;
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

    ((UnkState*)0x801ED500)->unk38 = (u32)DecDCToutCallback(&func_80140AC0, p1, p2, p3);
    ((UnkState*)0x801ED500)->unk3C = DrawSyncCallback(&func_801416C4);

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

    CD_WaitForQueueEmpty();
    new_var3 = (UnkState*)0x801ED500;
    CD_QueueCommand(0x1B, (s16)arg0, (void*)0, &func_80140F04);

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