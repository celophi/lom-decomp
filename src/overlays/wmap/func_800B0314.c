#include "common.h"

extern s32 D_801B2F40;

/**
 * @brief Increment a world-map state counter.
 */
void func_800B0314(void)
{
    D_801B2F40 += 1;
}
