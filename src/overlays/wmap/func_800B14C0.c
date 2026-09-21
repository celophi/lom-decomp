#include "common.h"

extern s32 D_801B2F80;

/**
 * @brief Increment a world-map state counter.
 */
void func_800B14C0(void)
{
    D_801B2F80 += 1;
}
