#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B2B90;
extern s32 D_801B2B94;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_800962B0(void)
{
    D_801ADAE0 = 1;
    D_801B2B94 = 0x23;
    D_801B2B90 += 1;
}
