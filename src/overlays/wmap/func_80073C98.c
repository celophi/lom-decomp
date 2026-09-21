#include "common.h"

extern s32 D_801B2540;
extern s32 D_801B2544;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80073C98(void)
{
    D_801B2540 = 1;
    D_801B2544 = 1;
}
