#include "common.h"

extern s32 D_801B2E98;
extern s32 D_801B2E9C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACBDC(void)
{
    D_801B2E98 = 1;
    D_801B2E9C = 1;
}
