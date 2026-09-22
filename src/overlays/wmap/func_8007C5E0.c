#include "common.h"

extern s32 D_801B2700;

/**
 * @brief Increment a world-map state counter.
 */
void func_8007C5E0(void)
{
    D_801B2700 += 1;
}
