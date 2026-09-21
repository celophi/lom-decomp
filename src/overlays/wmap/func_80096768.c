#include "common.h"

extern s32 D_801B2BA0;
extern s32 D_801B2BA4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80096768(void)
{
    D_801B2BA0 = 1;
    D_801B2BA4 = 1;
}
