#include "common.h"

extern s32 D_801B2888;

/**
 * @brief Increment a world-map state counter.
 */
void func_80084D18(void)
{
    D_801B2888 += 1;
}
