#include "common.h"

extern s32 D_801B29B8;
extern s32 D_801B29BC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008BA04(void)
{
    D_801B29B8 = 1;
    D_801B29BC = 1;
}
