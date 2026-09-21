#include "common.h"

extern s32 D_801B2D50;
extern s32 D_801B2D54;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A167C(void)
{
    D_801B2D50 = 1;
    D_801B2D54 = 1;
}
