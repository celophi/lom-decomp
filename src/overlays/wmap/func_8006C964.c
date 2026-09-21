#include "common.h"

extern s32 D_801B1098;
extern s32 D_801B109C;

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_8006C964(void)
{
    D_801B109C = 4;
    D_801B1098 += 1;
}
