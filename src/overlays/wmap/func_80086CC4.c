#include "common.h"

extern s32 D_801B28F8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80086CC4(void)
{
    D_801B28F8 += 1;
}
