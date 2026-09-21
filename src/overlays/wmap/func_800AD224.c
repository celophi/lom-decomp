#include "common.h"

extern s32 D_801B2EC0;

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD224(void)
{
    D_801B2EC0 += 1;
}
