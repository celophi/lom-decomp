#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ShopRect;

extern u8 D_800EC3D0[];
extern u8 D_800EC3FE[];
extern s32 D_8014523C;
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);

s32 func_80142284(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    ShopRect pos;

    switch (D_8014523C)
    {
    case 0:
        prim = func_800A88A0(prim, ot,
            (void *)(D_800EC3D0 - 0xC + D_800EC3D0[0] + (D_800EC3D0[1] << 8)),
            6, 0x80 - arg2, -arg3, 2);
        break;
    case 1:
        prim = func_800A88A0(prim, ot,
            (void *)(D_800EC3FE - 0x3A + D_800EC3FE[0] + (D_800EC3FE[1] << 8)),
            6, 0x80 - arg2, -arg3, 2);
        break;
    }
    return prim;
}
