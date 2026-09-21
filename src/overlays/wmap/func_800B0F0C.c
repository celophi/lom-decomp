#include "common.h"

extern s32 D_801B2F70;

/**
 * @brief Increment a world-map state counter.
 */
void func_800B0F0C(void)
{
    D_801B2F70 += 1;
}
