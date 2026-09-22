#include "common.h"

extern s32 D_801B2610;

/**
 * @brief Increment a world-map state counter.
 */
void func_80078110(void)
{
    D_801B2610 += 1;
}
