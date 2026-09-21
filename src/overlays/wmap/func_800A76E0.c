#include "common.h"

extern s32 D_801B2E40;
extern s32 D_801B2E44;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A76E0(void)
{
    D_801B2E40 = 1;
    D_801B2E44 = 1;
}
