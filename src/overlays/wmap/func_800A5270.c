#include "common.h"

extern s32 D_801B2E18;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5270(void)
{
    D_801B2E18 += 1;
}
