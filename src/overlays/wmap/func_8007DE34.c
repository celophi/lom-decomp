#include "common.h"

extern s32 D_801B2748;
extern s32 D_801B274C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007DE34(void)
{
    D_801B2748 = 1;
    D_801B274C = 1;
}
