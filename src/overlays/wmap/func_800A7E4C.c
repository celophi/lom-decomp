#include "common.h"

extern s32 D_801B2E48;
extern s32 D_801B2E4C;

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_800A7E4C(void)
{
    D_801B2E4C = 0x3C;
    D_801B2E48 += 1;
}
