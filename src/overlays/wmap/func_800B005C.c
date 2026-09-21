#include "common.h"

extern s32 D_801B2F30;

/**
 * @brief Increment a world-map state counter.
 */
void func_800B005C(void)
{
    D_801B2F30 += 1;
}
