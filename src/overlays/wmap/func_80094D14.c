#include "common.h"

extern s32 D_801B2B50;

/**
 * @brief Increment a world-map state counter.
 */
void func_80094D14(void)
{
    D_801B2B50 += 1;
}
