#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct AddheroPacket {
    s32 attr;
    s32 flags;
    s32 draw;
} AddheroPacket;

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

extern s32 D_801609E8;
extern u16 D_80146FE0;
extern u16 D_80146FE2;
extern u16 D_80146FE4;
extern u16 D_80146FE6;
extern s32 D_80122988;
extern s32 D_8012298C;
extern s32 D_8016092C;
extern AddheroPacket D_80160940;

/** @see decomp.me (100%) */
s32 func_80142E6C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    AddheroPacket *p;
    s32 i;

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
        D_8016092C = 3;
        D_8012298C = 0x20;
        p = &D_80160940;
        for (i = 0; i < 8; i++)
        {
            p->flags &= ~0x200;
            p->attr &= ~7;
            p++;
        }
        func_80067F5C(8);
        func_800AA02C();
    }
    return prim;
}
