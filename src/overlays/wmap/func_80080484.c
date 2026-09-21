#include "common.h"

extern s32 D_801B27B8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80080484(void)
{
    D_801B27B8 += 1;
}
