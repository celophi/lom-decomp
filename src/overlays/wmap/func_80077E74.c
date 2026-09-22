#include "common.h"

extern s32 D_801B2600;

/**
 * @brief Increment a world-map state counter.
 */
void func_80077E74(void)
{
    D_801B2600 += 1;
}
