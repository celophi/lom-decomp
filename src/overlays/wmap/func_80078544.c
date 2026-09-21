#include "common.h"

extern s32 D_801B2628;

/**
 * @brief Increment a world-map state counter.
 */
void func_80078544(void)
{
    D_801B2628 += 1;
}
