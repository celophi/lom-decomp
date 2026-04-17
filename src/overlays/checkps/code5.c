#include "checkps.h"

/**
 * GNU AS (99.91%) match
 */
s32 PollCdResponse(s32 arg0)
{
    u8 t2;
    u8 val1;
    u8 val2;
    s32 temp_v1;
    s32 temp_a2;
    s32 i;
    int new_var;
    s32 j;
    t2 = g_cdCmdTable[arg0].irqThresh;
    *g_cdStatusRegister = 1;
    val1 = *((volatile u8*)g_cdIrqRegister);
    val2 = *((volatile u8*)g_cdIrqRegister);
    if ((new_var = val1 & 7) == (val2 & 7))
    {
        temp_v1 = new_var;
        temp_a2 = (unsigned char)temp_v1;
        if (temp_a2 != 0)
        {
            g_cdIrqAccum = g_cdIrqAccum + temp_a2;
            *g_cdStatusRegister = 1;
            *g_cdIrqRegister = 7;
            i = 0;
            do
            {
                *((int*)0) = i;
                i++;
            } while (i < 4);
            if (g_cdIrqAccum >= ((s32)t2))
            {
                j += 0;
                g_cdIrqAccum = 0;
                if (temp_v1 == 5)
                {
                    do
                    {
                        g_statusFlag.unk0 = *g_cdResponseRegister;
                    } while (0);
                    g_statusFlag.unk1 = *g_cdResponseRegister;
                    *g_cdStatusRegister = 1;
                    *g_cdDataRegister = 0x1F;
                    if (!(g_statusFlag.unk0 & 0x10))
                    {
                        return -1;
                    }

                    return -2;
                }
                else
                {
                    temp_a2 = 0;
                    j = temp_a2;
                    if (g_cdCmdTable[arg0].respCount != temp_a2)
                    {
                        do
                        {
                            ((u8*)(&g_statusFlag))[j] = *g_cdResponseRegister;
                            j++;
                        } while (j < ((s32)(*((volatile u8*)(&g_cdCmdTable[arg0].respCount)))));
                    }
                    *g_cdStatusRegister = 1;
                    *g_cdDataRegister = 0x1F;
                    if (arg0 != 0xA)
                    {
                        j = g_statusFlag.unk0;
                        if (j & 0x10)
                        {
                            return -2;
                        }
                    }
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}

/**
 * decomp.me link (83.42%%) https://decomp.me/scratch/vhWhP
 * Matches 100% with GNU AS
 */
void SendCdCommand(int arg0)
{
    s32 i = 0;
    s32* ptr = 0;
    s32 j;

    *g_cdStatusRegister = 1;
    *g_cdIrqRegister = 7;

    for (i = 0; i < 4; i++)
        *ptr = i;

    *g_cdStatusRegister = 1;
    *g_cdDataRegister = 0x18;
    *g_cdStatusRegister = 0;

    j = 0;
    if (g_cdCmdTable[arg0].paramCount)
    {
        do
        {
            *g_cdDataRegister = g_CmdBuf[j];
            j++;
        } while (j < g_cdCmdTable[arg0].paramCount);
    }

    *g_cdStatusRegister = 0;
    *g_cdResponseRegister = g_cdCmdTable[arg0].opcode;
}

/**
 * decomp.me link (96.94%) https://decomp.me/scratch/rZ9Jk
 * Matches 100% with GNU AS
 */
void ExitCheckPS(void)
{
    DRAWENV sp18;
    DISPENV sp78;
    DR_ENV sp90;
    unsigned long new_var;
    u32 spD0[3];
    FourShorts spE;
    s32 var_a2;
    s32 var_s0;
    ResetGraph(1);
    StopCallback();
    ResetGraph(5);
    *((s16*)0x1F801DAA) = 0;
    SetDefDrawEnv(&sp18, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(&sp78, 0, 0, 0x140, 0xF0);
    sp18.isbg = 1;
    SetDrawEnv(&sp90, &sp18);
    DrawPrim(&sp90);
    PutDispEnv(&sp78);
    spD0[0] = 0x02000000;
    spD0[1] = 0xE6000002;
    spD0[2] = 0;
    DrawPrim(spD0);
    var_a2 = 0xFFFF;

    spE.c = 0x10;
    spE.d = 1;

    for (var_s0 = 0; var_s0 < 2; var_s0++)
    {
        spE.a = var_s0 + 0x50;
        spE.b = var_s0 + 0x5C;
        func_80051830((u32)(&D_8004FCC4), &spE, var_a2);
        var_a2 = 0x8000;
    }

    DrawSymmetricTestPattern();
    SetDispMask(1);
    exit();
}