#include "common.h"

extern s32 D_801AFBA0;
extern s32 D_801AFBA4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800654BC(void)
{
    D_801AFBA0 = 1;
    D_801AFBA4 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800654D4(void)
{
    D_801AFBA0 += 1;
}
