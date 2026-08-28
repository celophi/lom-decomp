#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void (*draw_handler)();
} CardaPacket;

extern u8 D_800EC3D0[];
extern s32 D_8012298C;
extern s32 D_80165F80;

s32 func_8014256C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    return func_800A88A0(prim, ot,
        (void *)((u8 *)D_800EC3D0 - 0xC + D_800EC3D0[0] + (D_800EC3D0[1] << 8)),
        5, 0x80 - arg2, -arg3, 2);
}

void func_801425D4(void)
{
    CardaPacket *p;
    s32 i;

    D_8012298C = 0x20;
    p = (CardaPacket *)&D_80165F80;
    for (i = 0; i < 8; i++)
    {
        p->attr.word &= ~7;
        p++;
    }
}
