#include "common.h"

extern s32 D_801B2BE8;
extern s32 D_801B2BEC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80097E70(void)
{
    D_801B2BE8 = 1;
    D_801B2BEC = 1;
}
