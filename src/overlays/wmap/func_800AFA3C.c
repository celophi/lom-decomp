#include "common.h"

extern s32 D_801B2F10;
extern s32 D_801B2F14;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AFA3C(void)
{
    D_801B2F10 = 1;
    D_801B2F14 = 1;
}
