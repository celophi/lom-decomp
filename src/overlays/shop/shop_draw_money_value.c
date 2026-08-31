#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
} ShopVec2s;

extern u8 D_800EC3C4[];
extern void *D_8012271C;
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);
extern s32 func_800A8A78(s32 *, s32, u32, s32, ShopVec2s *, s32);

s32 func_801414F8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    ShopVec2s pos;
    s32 y;

    y = 2 - arg3;
    prim = func_800A88A0(prim, ot,
        (void *)(D_800EC3C4 + D_800EC3C4[0] + (D_800EC3C4[1] << 8)),
        4, 0x50 - arg2, y, 0);
    pos.x = 0x30 - arg2;
    pos.y = y;
    if (*(u32 *)((u8 *)D_8012271C + 0x2C) > 0x989680U)
    {
        prim = func_800A8A78(ot, prim, 0x989680U, 4, &pos, 2);
    }
    else
    {
        prim = func_800A8A78(ot, prim, *(u32 *)((u8 *)D_8012271C + 0x2C), 4, &pos, 2);
    }
    return prim;
}
