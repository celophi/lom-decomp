#include "common.h"

extern s32 D_801B2530;

/**
 * @brief Increment a world-map state counter.
 */
void func_80073AC4(void)
{
    D_801B2530 += 1;
}
