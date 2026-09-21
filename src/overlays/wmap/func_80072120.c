#include "common.h"

extern s32 D_801B24E8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80072120(void)
{
    D_801B24E8 += 1;
}
