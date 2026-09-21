#include "common.h"

extern s32 D_801B2EA8;
extern s32 D_801B2EAC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACE24(void)
{
    D_801B2EA8 = 1;
    D_801B2EAC = 1;
}
