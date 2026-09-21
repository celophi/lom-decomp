#include "common.h"

extern s32 D_801B2A38;

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E3E0(void)
{
    D_801B2A38 += 1;
}
