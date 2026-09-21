#include "common.h"

extern s32 D_801B2590;

/**
 * @brief Increment a world-map state counter.
 */
void func_80075DC0(void)
{
    D_801B2590 += 1;
}
