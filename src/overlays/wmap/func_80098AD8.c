#include "common.h"

extern s32 D_801B2C08;

/**
 * @brief Increment a world-map state counter.
 */
void func_80098AD8(void)
{
    D_801B2C08 += 1;
}
