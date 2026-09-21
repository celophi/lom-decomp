#include "common.h"

extern s32 D_801B2528;
extern s32 D_801B252C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800734A0(void)
{
    D_801B2528 = 1;
    D_801B252C = 1;
}
