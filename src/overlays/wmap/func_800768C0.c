#include "common.h"

extern s32 D_801B25C8;
extern s32 D_801B25CC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800768C0(void)
{
    D_801B25C8 = 1;
    D_801B25CC = 1;
}
