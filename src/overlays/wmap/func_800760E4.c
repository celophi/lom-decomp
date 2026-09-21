#include "common.h"

extern s32 D_801B25A0;
extern s32 D_801B25A4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800760E4(void)
{
    D_801B25A0 = 1;
    D_801B25A4 = 1;
}
