#include "common.h"

extern s32 D_801B27D8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80080DAC(void)
{
    D_801B27D8 += 1;
}
