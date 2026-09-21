#include "common.h"

extern s32 D_801B2910;
extern s32 D_801B2914;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800877BC(void)
{
    D_801B2910 = 1;
    D_801B2914 = 1;
}
