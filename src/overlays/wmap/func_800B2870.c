#include "common.h"

extern s32 D_801B2FA0;
extern s32 D_801B2FA4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2870(void)
{
    D_801B2FA0 = 1;
    D_801B2FA4 = 1;
}
