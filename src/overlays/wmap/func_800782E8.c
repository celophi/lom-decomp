#include "common.h"

extern s32 D_801B2620;
extern s32 D_801B2624;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800782E8(void)
{
    D_801B2620 = 1;
    D_801B2624 = 1;
}
