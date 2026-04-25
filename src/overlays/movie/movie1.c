#include "movie.h"

/**
 * decomp.me link (97.51%) https://decomp.me/scratch/XvMvo
 */
void FUN_80140018(unsigned short arg0)
{
    DISPENV env1;
    DISPENV env2;
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
    SetDefDispEnv(&env1, 0, 0, 320, timeout);
    SetDefDispEnv(&env2, 0, timeout, 320, timeout);
    env1.isrgb24 = (env2.isrgb24 = 1);
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

        func_80140358((s32)((arg0 & 0xFFFF) + 0x16A0), 0x80, (s32)a2, 0);
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
    new_var5 = &env1;
    if (p500->unk98 == 0)
    {
        new_var5 = &env2;
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
            timeout = 0x2000;
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