#include "common.h"

extern s32 D_801B2920;

/**
 * @brief Increment a world-map state counter.
 */
void func_80087EB8(void)
{
    D_801B2920 += 1;
}
