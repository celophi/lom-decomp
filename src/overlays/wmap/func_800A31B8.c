#include "common.h"

extern s32 D_801B2DC8;
extern s32 D_801B2DCC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A31B8(void)
{
    D_801B2DC8 = 1;
    D_801B2DCC = 1;
}
