#include "common.h"

extern s32 D_801B2F38;

/**
 * @brief Increment a world-map state counter.
 */
void func_800B01B8(void)
{
    D_801B2F38 += 1;
}
