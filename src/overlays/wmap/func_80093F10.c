#include "common.h"

extern s32 D_801B2B28;
extern s32 D_801B2B2C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80093F10(void)
{
    D_801B2B28 = 1;
    D_801B2B2C = 1;
}
