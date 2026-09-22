#include "common.h"

extern s32 D_800DCEC0;
extern s32 D_8011CF50;
extern s32 D_8013B208;
extern s32 D_801B2E40;

/** @brief Reset two world-map values, set the enable flag, and advance the state. */
void func_800A7BB8(void)
{
    D_8011CF50 = 0;
    D_8013B208 = 0;
    D_800DCEC0 = 1;
    D_801B2E40 += 1;
}
