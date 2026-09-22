#include "common.h"

extern s32 D_801B2D80;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2490(void)
{
    D_801B2D80 += 1;
}
