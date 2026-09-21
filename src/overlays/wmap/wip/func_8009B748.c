#include "common.h"

extern void func_8009D0FC(void);
extern u8 *D_80139280;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern u8 D_80121538;
extern s32 D_801B2CB8;
extern s32 D_801B2CBC;

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance.
 *  @note Best match ~85.6% (gcc280_g0); residual is loop induction-variable register coloring. */
void func_8009B748(void)
{
    u8 *pa;
    u8 *pb;
    u8 *p;
    s32 i;
    s32 off_a;
    s32 off_b;

    p = D_80139280;
    *(s32 *)(p + 0x5C) = 0x20;
    *(s32 *)(p + 0x68) = 0x3E8;
    *(s32 *)(p + 0x6C) = 0x64;
    *(s32 *)(p + 0x70) = 8;
    *(s32 *)(p + 0x74) = 2;
    *(s32 *)(p + 0x54) = 1;
    *(s32 *)(p + 0x58) = 1;
    *(s32 *)(p + 0x60) = 0;
    *(s32 *)(p + 0x64) = 1;
    *(s32 *)(p + 0x78) = 0x6D60;

    pa = &D_80139988;
    pb = &D_801AFBD0;
    i = 0;
    off_a = 0x320;
    off_b = 0x7D0;
    do
    {
        *(s16 *)(pb + off_b) = 0;
        *(s32 *)(pa + off_a + 4) = (s32)&D_80121538;
        off_a += 8;
        off_b += 0x14;
        i += 1;
    } while (i < 0x14);

    D_801B2CBC = 0x14;
    D_801B2CB8 += 1;
    func_8009D0FC();
}
