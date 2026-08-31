#include "common.h"

typedef struct { s16 x; s16 y; s16 w; s16 h; } RECT;

typedef struct CardaElement {
    union {
        u32 word;
        struct {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4;
    void *draw_handler;
    s32 unkC;
} CardaElement;

extern s32 D_80122988;
extern u16 D_8014B058;
extern CardaElement D_80165F80;
extern s32 D_80165FEC;

s32 func_80143F90(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;

    result = func_800A88A0(prim, ot, (void *)((s32)&D_8014B058 - 0x20 + D_8014B058), 4,
                            -arg2 + 0x90, -arg3, 2);
    if (D_80122988 & 0x260)
    {
        func_800A3938(0x7D, 0x80);
        D_80165F80.attr.f.state = 0;
        func_800AA02C();
    }
    else if (D_80165FEC == 0xFD)
    {
        D_80165F80.attr.f.state = 0;
        func_800AA02C();
    }
    return result;
}
