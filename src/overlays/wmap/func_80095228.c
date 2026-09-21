#include "common.h"

extern s32 D_801B2B68;
extern s32 D_801B2B6C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80095228(void)
{
    D_801B2B68 = 1;
    D_801B2B6C = 1;
}
