#include "common.h"
#include "main.h"
#include "sdk/libgpu.h"

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

typedef struct {
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
    void (*draw_handler)();
} CardaPacket;

extern s32 D_80165FE4;
extern u16 D_8014B074;
extern u16 D_8014B076;
extern u16 D_8014B078;
extern u16 D_8014B07A;
extern s32 D_80165F80;
extern s32 D_80122988;
extern s32 D_8012298C;

s32 func_80146AF0(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    CardaPacket *p;
    s32 i;

    switch (D_80165FE4)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B074, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B078, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B076, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (D_80122988 & 0x220)
    {
        D_8012298C = 0x20;
        p = (CardaPacket *)&D_80165F80;
        for (i = 0; i < 8; i++)
        {
            p->attr.word &= ~7;
            p++;
        }
        func_80067F5C(8);
        func_800AA02C();
    }
    return prim;
}
