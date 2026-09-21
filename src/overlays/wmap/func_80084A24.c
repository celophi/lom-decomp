#include "common.h"

extern s32 D_801B2878;

/**
 * @brief Increment a world-map state counter.
 */
void func_80084A24(void)
{
    D_801B2878 += 1;
}
