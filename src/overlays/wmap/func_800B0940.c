#include "common.h"

extern s32 D_801B2F58;

/**
 * @brief Increment a world-map state counter.
 */
void func_800B0940(void)
{
    D_801B2F58 += 1;
}
