#include "common.h"

extern s32 D_801B2648;

/**
 * @brief Increment a world-map state counter.
 */
void func_80078E60(void)
{
    D_801B2648 += 1;
}
