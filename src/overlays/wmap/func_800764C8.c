#include "common.h"

extern s32 D_801B25B8;
extern s32 D_801B25BC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800764C8(void)
{
    D_801B25B8 = 1;
    D_801B25BC = 1;
}
