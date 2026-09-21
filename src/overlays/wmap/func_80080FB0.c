#include "common.h"

extern s32 D_801B27E0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80080FB0(void)
{
    D_801B27E0 += 1;
}
