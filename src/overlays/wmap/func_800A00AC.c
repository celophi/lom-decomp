#include "common.h"

extern s32 D_801B2D30;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A00AC(void)
{
    D_801B2D30 += 1;
}
