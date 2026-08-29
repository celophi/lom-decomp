#include "common.h"
typedef struct { s16 x; s16 y; s16 w; s16 h; } RECT;
extern u16 D_8014B0BE;
s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
s32 func_80146E80(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    return func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BE, 0x86), 4, -x_offset + 0x80, -y_offset, 2);
}
