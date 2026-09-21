#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B2BE8;
extern s32 D_801B2BEC;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_80097FFC(void)
{
    D_801ADAE0 = 1;
    D_801B2BEC = 0x28;
    D_801B2BE8 += 1;
}
