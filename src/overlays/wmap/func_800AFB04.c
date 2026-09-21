#include "common.h"

extern s32 D_801B2F10;

/**
 * @brief Increment a world-map state counter.
 */
void func_800AFB04(void)
{
    D_801B2F10 += 1;
}
