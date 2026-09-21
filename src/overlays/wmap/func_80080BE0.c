#include "common.h"

extern s32 D_801B27D0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80080BE0(void)
{
    D_801B27D0 += 1;
}
