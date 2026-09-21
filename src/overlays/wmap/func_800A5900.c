#include "common.h"

extern s32 D_801B2E28;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5900(void)
{
    D_801B2E28 += 1;
}
