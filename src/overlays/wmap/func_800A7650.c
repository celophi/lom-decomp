#include "common.h"

extern s32 D_801B2E70;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A7650(void)
{
    D_801B2E70 += 1;
}
