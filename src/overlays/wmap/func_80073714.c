#include "common.h"

extern s32 D_801B2518;
extern s32 D_801B251C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80073714(void)
{
    D_801B2518 = 1;
    D_801B251C = 1;
}
