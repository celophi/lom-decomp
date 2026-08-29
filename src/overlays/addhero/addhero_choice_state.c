#include "common.h"
#include "main.h"
#include "sdk/libgpu.h"

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

typedef struct AddheroElement {
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
} AddheroElement;

extern s32 D_801609E8;
extern u16 D_80146FE0;
extern u16 D_80146FE2;
extern u16 D_80146FE4;
extern u16 D_80146FE6;
extern AddheroElement D_80160940;
extern s32 D_80122988;

s32 func_80142CE8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    switch (D_801609E8)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE0, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE4, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE6, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE2, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (D_80122988 & 0x220)
    {
        D_80160940.attr.f.state = 0;
        func_800AA02C();
    }
    return prim;
}
