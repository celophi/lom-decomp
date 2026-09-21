#include "common.h"

extern s32 D_801B2B00;

/**
 * @brief Increment a world-map state counter.
 */
void func_80092B30(void)
{
    D_801B2B00 += 1;
}
