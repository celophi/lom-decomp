#include "common.h"

extern s32 D_801B2880;

/**
 * @brief Increment a world-map state counter.
 */
void func_80084BC4(void)
{
    D_801B2880 += 1;
}
