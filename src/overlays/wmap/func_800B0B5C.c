#include "common.h"

extern s32 D_801B2F60;

/**
 * @brief Increment a world-map state counter.
 */
void func_800B0B5C(void)
{
    D_801B2F60 += 1;
}
