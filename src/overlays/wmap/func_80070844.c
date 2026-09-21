#include "common.h"

extern s32 D_801B2408;

/**
 * @brief Increment a world-map state counter.
 */
void func_80070844(void)
{
    D_801B2408 += 1;
}
