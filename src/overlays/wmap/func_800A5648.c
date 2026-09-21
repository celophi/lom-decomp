#include "common.h"

extern s32 D_801B2E28;
extern s32 D_801B2E2C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A5648(void)
{
    D_801B2E28 = 1;
    D_801B2E2C = 1;
}
