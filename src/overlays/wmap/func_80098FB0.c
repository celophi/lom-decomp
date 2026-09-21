#include "common.h"

extern s32 D_801B2C20;

/**
 * @brief Increment a world-map state counter.
 */
void func_80098FB0(void)
{
    D_801B2C20 += 1;
}
