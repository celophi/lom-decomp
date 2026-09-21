#include "common.h"

extern s32 D_801B2B90;
extern s32 D_801B2B94;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80096154(void)
{
    D_801B2B90 = 1;
    D_801B2B94 = 1;
}
