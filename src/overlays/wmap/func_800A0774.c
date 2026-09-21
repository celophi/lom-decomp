#include "common.h"

extern s32 D_801B2D48;
extern s32 D_801B2D4C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A0774(void)
{
    D_801B2D48 = 1;
    D_801B2D4C = 1;
}
