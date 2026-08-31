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
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void *draw_handler;
    s32 unkC;
} CardaElement;

extern u16 D_8014B054;
extern CardaElement D_80165F80;
extern s32 D_80165FEC;
extern s32 D_80166118;
extern void func_80143F90(void);

s32 func_8014385C(s32 result, s32 *ot);

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

s32 func_80143DF4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 x;
    s32 result;
    u8 *base;
    CardaElement *p;
    RECT pos;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_8014B054 - 0x1C + D_8014B054), 4, x, -arg3, 2);
    base = (u8 *)&D_8014B054 - 0x1C;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = func_8014385C(result, ot);

    if (D_80166118 == 0)
    {
        func_800A3938(0x7A, 0x80);
        D_80165FEC = 0xFF;
        p = &D_80165F80;
        p->draw_handler = func_80143F90;
        p->attr.f.unk0_3 = 1;
        p->attr.f.state = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x68;
        p->unk4_0 = 1;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x20);
    }
    return result;
}
