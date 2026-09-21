#include "common.h"

extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2F90;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_800B2218(void)
{
    D_8013B294 = 1;
    D_80139228 = 0x2;
    D_801B2F90 += 1;
}
