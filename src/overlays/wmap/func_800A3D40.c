#include "common.h"

extern s32 D_801B2DD0;
extern s32 D_801B2DD4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A3D40(void)
{
    D_801B2DD0 = 1;
    D_801B2DD4 = 1;
}
