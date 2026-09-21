#include "common.h"

extern s32 D_801B2820;

/**
 * @brief Increment a world-map state counter.
 */
void func_80082D58(void)
{
    D_801B2820 += 1;
}
