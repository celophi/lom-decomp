#include "common.h"

extern s32 D_801B2F90;
extern s32 D_801B2F94;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B20F8(void)
{
    D_801B2F90 = 1;
    D_801B2F94 = 1;
}
