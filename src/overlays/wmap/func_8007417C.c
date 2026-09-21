#include "common.h"

extern s32 D_801B2550;

/**
 * @brief Increment a world-map state counter.
 */
void func_8007417C(void)
{
    D_801B2550 += 1;
}
