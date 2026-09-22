#include "common.h"

extern s32 D_801B2D70;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2198(void)
{
    D_801B2D70 += 1;
}
