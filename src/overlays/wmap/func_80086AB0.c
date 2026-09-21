#include "common.h"

extern s32 D_801B28F0;
extern s32 D_801B28F4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80086AB0(void)
{
    D_801B28F0 = 1;
    D_801B28F4 = 1;
}
