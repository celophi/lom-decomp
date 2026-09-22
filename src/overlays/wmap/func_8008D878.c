#include "common.h"

extern s32 D_80139244;
extern s32 D_801B2A10;
extern s32 D_801B2A14;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8008D878(void)
{
    D_80139244 = 1;
    D_801B2A14 = 0x16;
    D_801B2A10 += 1;
}
