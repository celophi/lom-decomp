#include "common.h"

extern s32 D_801B25C8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80076AD0(void)
{
    D_801B25C8 += 1;
}
