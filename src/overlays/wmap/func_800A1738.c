#include "common.h"

extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2D50;

/** @brief World-map trigger: set two flags and bump a counter. */
void func_800A1738(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B2D50 += 1;
}
