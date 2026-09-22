#include "common.h"

extern s32 D_801B2B40;

/**
 * @brief Increment a world-map state counter.
 */
void func_80094A68(void)
{
    D_801B2B40 += 1;
}
