/* Partial WMAP decompilation: 88.215680% (gcc280_g0). */
#include "common.h"

extern void func_800A31D0(void);
extern u8 *D_80139280;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern u8 D_8011D538;
extern s32 D_801B2DC8;
extern s32 D_801B2DCC;

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance.
 *  @note Best match ~88% (gcc280_g0); residual is loop induction-variable register coloring. */
void func_800A1538(void)
{
    u8 *pa;
    u8 *pb;
    u8 *p;
    s32 i;
    s32 off_a;
    s32 off_b;

    p = D_80139280;
    *(s32 *)(p + 0x80) = 5;
    *(s32 *)(p + 0x84) = 0x20;
    *(s32 *)(p + 0x8C) = 8;
    *(s32 *)(p + 0x94) = 0x8C;
    *(s32 *)(p + 0x98) = 0x15;
    *(s32 *)(p + 0x7C) = 1;
    *(s32 *)(p + 0x88) = 0;
    *(s32 *)(p + 0x90) = 0;
    *(s32 *)(p + 0x9C) = 1;
    *(s32 *)(p + 0xA0) = 0x1F40;

    pa = &D_80139988;
    pb = &D_801AFBD0;
    i = 0;
    off_a = 0x480;
    off_b = 0xAF0;
    do
    {
        *(s16 *)(pb + off_b) = 0;
        *(s32 *)(pa + off_a + 4) = (s32)&D_8011D538;
        off_a += 8;
        off_b += 0x14;
        i += 1;
    } while (i < 0xA);

    D_801B2DCC = 0x50;
    D_801B2DC8 += 1;
    func_800A31D0();
}
