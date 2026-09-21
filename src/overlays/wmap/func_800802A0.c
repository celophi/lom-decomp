#include "common.h"

extern s32 D_801B27B8;
extern s32 D_801B27BC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800802A0(void)
{
    D_801B27B8 = 1;
    D_801B27BC = 1;
}
