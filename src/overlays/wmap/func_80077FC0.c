#include "common.h"

extern s32 D_801B2608;

/**
 * @brief Increment a world-map state counter.
 */
void func_80077FC0(void)
{
    D_801B2608 += 1;
}
