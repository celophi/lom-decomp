#include "common.h"

extern s32 D_801B2E70;
extern s32 D_801B2E74;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A73E8(void)
{
    D_801B2E70 = 1;
    D_801B2E74 = 1;
}
