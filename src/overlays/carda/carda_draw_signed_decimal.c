#include "common.h"

extern u16 D_80165EFC[];
extern s32 func_8014A900(s32, s32 *, u8 *, s32, s32, s32, s32);

s32 func_8014A65C(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 buf[7];
    s32 first_digit;
    s32 magnitude;
    s32 negative;

    magnitude = value;
    if (magnitude < 0)
    {
        magnitude = -magnitude;
        negative = 1;
    }
    else
    {
        negative = 0;
    }
    buf[1] = D_80165EFC[magnitude / 10000];
    buf[2] = D_80165EFC[(magnitude % 10000) / 1000];
    buf[3] = D_80165EFC[(magnitude % 1000) / 100];
    buf[4] = D_80165EFC[(magnitude % 100) / 10];
    buf[5] = D_80165EFC[magnitude % 10];

    first_digit = 1;
    buf[6] = 0;

    while (first_digit < 5 && buf[first_digit] == 0x4F82)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = 0x5B81;
    }
    prim = func_8014A900(prim, ot, (u8 *)&buf[first_digit], x, y, palette, alignment);
    return prim;
}
