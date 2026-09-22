#include "common.h"

extern s32 D_801B2AD0;
extern s32 D_801B2AD4;

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_80091D9C(void)
{
    D_801B2AD4 = 5;
    D_801B2AD0 += 1;
}
