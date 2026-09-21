#include "common.h"

extern s32 D_801B27B0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80080210(void)
{
    D_801B27B0 += 1;
}
