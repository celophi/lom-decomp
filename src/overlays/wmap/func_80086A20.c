#include "common.h"

extern s32 D_801B28E8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80086A20(void)
{
    D_801B28E8 += 1;
}
