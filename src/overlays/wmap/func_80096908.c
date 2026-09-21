#include "common.h"

extern s32 D_801B2BA8;
extern s32 D_801B2BAC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80096908(void)
{
    D_801B2BA8 = 1;
    D_801B2BAC = 1;
}
