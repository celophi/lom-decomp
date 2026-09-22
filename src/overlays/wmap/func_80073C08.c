#include "common.h"

extern s32 D_801B2538;

/**
 * @brief Increment a world-map state counter.
 */
void func_80073C08(void)
{
    D_801B2538 += 1;
}
