#include "common.h"

extern s32 D_801B2DA0;
extern s32 D_801B2DA4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A299C(void)
{
    D_801B2DA0 = 1;
    D_801B2DA4 = 1;
}
