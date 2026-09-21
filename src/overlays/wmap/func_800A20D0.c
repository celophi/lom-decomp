#include "common.h"

extern s32 D_801B2D70;
extern s32 D_801B2D74;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A20D0(void)
{
    D_801B2D70 = 1;
    D_801B2D74 = 1;
}
