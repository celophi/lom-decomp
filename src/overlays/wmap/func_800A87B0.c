#include "common.h"

extern s32 D_8011CF50;
extern s32 D_8013B208;
extern s32 D_801B2E58;

/** @brief World-map step handler: clear two flags and advance the step counter. */
void func_800A87B0(void)
{
    D_8011CF50 = 0;
    D_8013B208 = 0;
    D_801B2E58 += 1;
}
