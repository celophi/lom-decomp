#include "common.h"

extern s32 D_801B2A90;
extern s32 D_801B2A94;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090B6C(void)
{
    D_801B2A90 = 1;
    D_801B2A94 = 1;
}
