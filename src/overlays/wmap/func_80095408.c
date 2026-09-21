#include "common.h"

extern s32 D_801B2B68;

/**
 * @brief Increment a world-map state counter.
 */
void func_80095408(void)
{
    D_801B2B68 += 1;
}
