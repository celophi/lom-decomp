#include "common.h"

extern s32 D_801B2540;

/**
 * @brief Increment a world-map state counter.
 */
void func_80073D98(void)
{
    D_801B2540 += 1;
}
