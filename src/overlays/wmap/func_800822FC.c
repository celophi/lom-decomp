/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */
#include "common.h"

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B27F0;

void func_800822FC(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B27F0 += 1;
}
