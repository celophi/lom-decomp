#include "common.h"

extern s32 D_801B2460;

/**
 * @brief Increment a world-map state counter.
 */
void func_80070698(void)
{
    D_801B2460 += 1;
}
