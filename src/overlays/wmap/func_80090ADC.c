#include "common.h"

extern s32 D_801B2A88;

/**
 * @brief Increment a world-map state counter.
 */
void func_80090ADC(void)
{
    D_801B2A88 += 1;
}
