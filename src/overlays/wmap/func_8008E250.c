#include "common.h"

extern s32 D_801B2A30;

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E250(void)
{
    D_801B2A30 += 1;
}
