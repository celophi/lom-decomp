#include "common.h"

extern s32 D_801B2A80;
extern s32 D_801B2A84;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090694(void)
{
    D_801B2A80 = 1;
    D_801B2A84 = 1;
}
