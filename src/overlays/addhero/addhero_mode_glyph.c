#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} AddheroRect;

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

extern s32 D_8016093C;
extern u16 D_80146FEA;
extern u16 D_80146FE8;
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);

s32 func_80141430(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;

    if (D_8016093C == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FEA, 0x46), 4, -arg2 + 0x78, -arg3, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE8, 0x44), 4, -arg2 + 0x78, -arg3, 2);
    }
    return prim;
}
