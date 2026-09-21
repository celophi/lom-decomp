#include "common.h"

extern s32 D_801B2710;

/**
 * @brief Increment a world-map state counter.
 */
void func_8007C880(void)
{
    D_801B2710 += 1;
}
