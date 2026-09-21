#include "common.h"

extern s32 D_801B2FA8;
extern s32 D_801B2FAC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2A14(void)
{
    D_801B2FA8 = 1;
    D_801B2FAC = 1;
}
