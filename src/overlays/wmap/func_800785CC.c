#include "common.h"

extern s32 D_801B2638;
extern s32 D_801B263C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800785CC(void)
{
    D_801B2638 = 1;
    D_801B263C = 1;
}
