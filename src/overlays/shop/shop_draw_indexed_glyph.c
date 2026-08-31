#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ShopRect;

extern u8 D_800EC3C4[];
extern s32 D_801451B8;
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);

s32 func_801420B8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    ShopRect pos;
    s32 idx;

    idx = D_801451B8 * 2;
    return func_800A88A0(prim, ot,
        (void *)(D_800EC3C4 + D_800EC3C4[idx] + (D_800EC3C4[idx + 1] << 8)),
        4, 0x38 - arg2, 2 - arg3, 2);
}
