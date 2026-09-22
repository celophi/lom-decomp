#include "common.h"

extern s32 D_801B2898;

/**
 * @brief Increment a world-map state counter.
 */
void func_80085060(void)
{
    D_801B2898 += 1;
}
