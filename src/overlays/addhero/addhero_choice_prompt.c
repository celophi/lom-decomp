#include "common.h"

extern s32 D_801609B0;
extern u8 D_800EC3FA[];
extern s32 D_80122988;

extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);
extern void func_800A3938(s32, s32);

void func_80144008(void)
{
    D_801609B0 = 1;
}

s32 func_80144018(s32 prim, s32 *ot, s32 x, s32 y)
{
    u8 *p;
    u8 *base;
    s32 g1;
    s32 g2;
    s32 hi;
    s32 a3;

    p = (u8 *)&D_800EC3FA;
    hi = p[1] << 8;
    base = p - 0x36;
    a3 = 4;
    g1 = p[0] + (hi + (s32)base);
    if (D_801609B0 != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (D_801609B0 == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g2, a3, x + 8, y, 0);
    if (D_80122988 & 0xA000)
    {
        D_801609B0 ^= 1;
        func_800A3938(0x7D, 0x80);
        D_80122988 = 0;
    }
    return prim;
}
