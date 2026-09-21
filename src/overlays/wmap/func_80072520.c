#include "common.h"

extern s32 D_801B2500;

/**
 * @brief Increment a world-map state counter.
 */
void func_80072520(void)
{
    D_801B2500 += 1;
}
