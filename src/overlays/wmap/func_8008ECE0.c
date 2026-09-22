#include "common.h"

extern s32 D_801B2A60;

/**
 * @brief Increment a world-map state counter.
 */
void func_8008ECE0(void)
{
    D_801B2A60 += 1;
}
