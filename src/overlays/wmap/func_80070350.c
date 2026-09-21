#include "common.h"

extern s32 D_801B2450;
extern s32 D_801B2454;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80070350(void)
{
    D_801B2450 = 1;
    D_801B2454 = 1;
}
