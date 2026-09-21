#include "common.h"

extern s32 D_801B2F00;

/**
 * @brief Increment a world-map state counter.
 */
void func_800AF80C(void)
{
    D_801B2F00 += 1;
}
