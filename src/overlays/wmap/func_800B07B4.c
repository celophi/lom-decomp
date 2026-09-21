#include "common.h"

extern s32 D_801B2F58;
extern s32 D_801B2F5C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B07B4(void)
{
    D_801B2F58 = 1;
    D_801B2F5C = 1;
}
