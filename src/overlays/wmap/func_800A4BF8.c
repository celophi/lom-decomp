#include "common.h"

extern s32 D_801B2E00;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4BF8(void)
{
    D_801B2E00 += 1;
}
