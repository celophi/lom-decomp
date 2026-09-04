#include "common.h"
#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))
extern s32 D_80122988;
extern s32 D_8012299C;
extern u16 D_8014B074;
extern u16 D_8014B076;
extern u16 D_8014B078;
extern u16 D_8014B07A;
extern u16 D_8014B09C;
extern s32 D_80165F80;
extern s32 D_80165FE4;
extern s32 D_80165FEC;
extern s32 D_80166078;
extern void *jtbl_801400AC[];
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);
extern void func_800AA02C();

/**
 * @brief Draw the active CARDA status dialog and handle dismissal input.
 * @param ot Ordering table used by the text renderer.
 * @param prim Current primitive-buffer cursor.
 * @param x Horizontal transition offset.
 * @param y Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see matching: 100.00%
 */
s32 func_80144A24(s32 *ot, s32 prim, s32 x, s32 y)
{
    u8 *base;
    s32 frame_scratch[2];
    s32 state = D_80165FE4;
    static void *const keep[] = { &&case0, &&case1, &&case2, &&case3, &&case4, &&case5 };

    if ((u32)state >= 6)
        goto after_switch;
    goto *jtbl_801400AC[state];

case0:
    prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B074,0x3C),4,-x+0x80,-y,2);
    base=(u8*)&D_8014B074-0x3C;
    prim = func_800A88A0(prim, ot, GLYPH_OFF(base,0x56),4,-x+0x80,-y+0x10,2);
    goto after_switch;
case1:
    prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B076,0x3E),4,-x+0x80,-y,2);
    base=(u8*)&D_8014B076-0x3E;
    prim = func_800A88A0(prim, ot, GLYPH_OFF(base,0x56),4,-x+0x80,-y+0x10,2);
    goto after_switch;
case2:
    prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B078,0x40),4,-x+0x80,-y,2);
    goto after_switch;
case3:
    prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A,0x42),4,-x+0x80,-y,2);
    if (D_80165FEC != 0xFD) goto check_pad;
    goto clear_state;
case4:
    prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B076,0x3E),4,-x+0x80,-y,2);
    base=(u8*)&D_8014B076-0x3E;
    prim = func_800A88A0(prim, ot, GLYPH_OFF(base,0x5C),4,-x+0x80,-y+0x10,2);
    goto after_switch;
case5:
    prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B09C,0x64),4,-x+0x80,-y,2);
    base=(u8*)&D_8014B09C-0x64;
    prim = func_800A88A0(prim, ot, GLYPH_OFF(base,0x56),4,-x+0x80,-y+0x10,2);

after_switch:
check_pad:
    if (D_80122988 & 0x220) {
        switch (D_80166078) {
        case 2:
            D_8012299C = 6;
            break;
        case 3:
            D_8012299C = 7;
            break;
        default:
            D_8012299C = 3;
            break;
        }
clear_state:
        D_80165F80 &= ~7;
        func_800AA02C();
    }
    return prim;
}
