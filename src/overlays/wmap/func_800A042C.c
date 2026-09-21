#include "common.h"

extern s32 D_801B2D40;
extern s32 D_801B2D44;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A042C(void)
{
    D_801B2D40 = 1;
    D_801B2D44 = 1;
}
