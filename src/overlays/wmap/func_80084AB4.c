#include "common.h"

extern s32 D_801B2880;
extern s32 D_801B2884;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80084AB4(void)
{
    D_801B2880 = 1;
    D_801B2884 = 1;
}
