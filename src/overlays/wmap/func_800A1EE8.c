#include "common.h"

extern s32 D_801B2D60;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A1EE8(void)
{
    D_801B2D60 += 1;
}
