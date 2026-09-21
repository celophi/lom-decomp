#include "common.h"

extern s32 D_801B2B70;

/**
 * @brief Increment a world-map state counter.
 */
void func_80095514(void)
{
    D_801B2B70 += 1;
}
