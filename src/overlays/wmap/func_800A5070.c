#include "common.h"

extern s32 D_801B2E10;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5070(void)
{
    D_801B2E10 += 1;
}
