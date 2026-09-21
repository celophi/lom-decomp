#include "common.h"

extern s32 D_801B2F28;
extern s32 D_801B2F2C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AFE44(void)
{
    D_801B2F28 = 1;
    D_801B2F2C = 1;
}
