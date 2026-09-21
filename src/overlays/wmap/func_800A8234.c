#include "common.h"

extern s32 D_801B2E50;
extern s32 D_801B2E54;

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_800A8234(void)
{
    D_801B2E54 = 0x3C;
    D_801B2E50 += 1;
}
