#include "common.h"

extern s32 D_801B2EF8;
extern s32 D_801B2EFC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AECE8(void)
{
    D_801B2EF8 = 1;
    D_801B2EFC = 1;
}
