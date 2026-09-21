#include "common.h"

extern s32 D_801B2A98;

/**
 * @brief Increment a world-map state counter.
 */
void func_80090ECC(void)
{
    D_801B2A98 += 1;
}
