#include "common.h"

extern s32 D_801B2950;

/**
 * @brief Increment a world-map state counter.
 */
void func_80088920(void)
{
    D_801B2950 += 1;
}
