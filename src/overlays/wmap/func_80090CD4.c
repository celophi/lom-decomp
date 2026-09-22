#include "common.h"

extern s32 D_801B2A90;

/**
 * @brief Increment a world-map state counter.
 */
void func_80090CD4(void)
{
    D_801B2A90 += 1;
}
