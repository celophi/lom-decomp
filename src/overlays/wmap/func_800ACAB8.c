#include "common.h"

extern s32 D_801B2E90;
extern s32 D_801B2E94;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACAB8(void)
{
    D_801B2E90 = 1;
    D_801B2E94 = 1;
}
