#include "common.h"

extern s32 D_801B2B10;

/**
 * @brief Increment a world-map state counter.
 */
void func_80092FCC(void)
{
    D_801B2B10 += 1;
}
