#include "common.h"

extern s32 D_801B2D68;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2040(void)
{
    D_801B2D68 += 1;
}
