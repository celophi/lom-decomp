#include "common.h"

extern s32 D_801B2C00;

/**
 * @brief Increment a world-map state counter.
 */
void func_80098980(void)
{
    D_801B2C00 += 1;
}
