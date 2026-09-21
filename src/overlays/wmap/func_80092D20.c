#include "common.h"

extern s32 D_801B2B10;
extern s32 D_801B2B14;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80092D20(void)
{
    D_801B2B10 = 1;
    D_801B2B14 = 1;
}
