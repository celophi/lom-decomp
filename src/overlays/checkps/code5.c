#include "checkps.h"

/**
 * decomp.me link (83.42%%) https://decomp.me/scratch/vhWhP
 * Note the compiler or flags may be wrong.
 */
void func_80051620(int arg0)
{
    s32 i = 0;
    s32* ptr = 0;
    s32 j;
    unsigned int idx;

    *D_8005CFC8 = 1;
    *D_8005CFD4 = 7;

    for (i = 0; i < 4; i++)
        *ptr = i;

    *D_8005CFC8 = 1;
    *D_8005CFD0 = 0x18;
    *D_8005CFC8 = 0;

    idx = arg0 * 4;

    j = 0;
    if (D_8005CF91[idx])
    {
        do
        {
            *D_8005CFD0 = D_8005CFD8[j];
            j++;
        } while (j < D_8005CF91[idx]);
    }

    *D_8005CFC8 = 0;
    *D_8005CFCC = D_8005CF90[arg0 * 4];
}

/**
 * decomp.me link (96.94%) https://decomp.me/scratch/rZ9Jk
 */
void func_80051710(void)
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

    func_80051A24();
    SetDispMask(1);
    exit();
}