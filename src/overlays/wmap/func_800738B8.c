#include "common.h"

extern s32 D_801B2520;
extern s32 D_801B2524;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800738B8(void)
{
    D_801B2520 = 1;
    D_801B2524 = 1;
}
