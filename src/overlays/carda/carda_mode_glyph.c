#include "common.h"

extern s32 D_80166078;
extern u16 D_8014B042;
extern u16 D_8014B064;
extern u16 D_8014B0D4;
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);

#define CARDA_GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

/**
 * @brief Draw the mode-dependent memory-card status glyph.
 *
 * The small frame scratch preserves the original GCC 2.7.2 stack-frame
 * bucket used by this seven-argument draw call.
 * @see matching: 100.00%
 */
s32 func_80141B50(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 frame_scratch[2];

    if ((u32)(D_80166078 - 2) < 2)
    {
        prim = func_800A88A0(prim, ot, CARDA_GLYPH_SYM(D_8014B0D4, 0x9C),
                             4, -x_offset + 0x80, -y_offset, 2);
    }
    else if (D_80166078 == 1)
    {
        prim = func_800A88A0(prim, ot, CARDA_GLYPH_SYM(D_8014B064, 0x2C),
                             4, -x_offset + 0x38, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, CARDA_GLYPH_SYM(D_8014B042, 0xA),
                             4, -x_offset + 0x38, -y_offset, 2);
    }

    return prim;
}
