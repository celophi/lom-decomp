#include "common.h"

extern s32 D_801B2D90;
extern s32 D_801B2D94;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A265C(void)
{
    D_801B2D90 = 1;
    D_801B2D94 = 1;
}
