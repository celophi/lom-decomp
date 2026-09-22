#include "common.h"

extern s32 D_801B2B60;

/**
 * @brief Increment a world-map state counter.
 */
void func_80095198(void)
{
    D_801B2B60 += 1;
}
