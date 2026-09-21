#include "common.h"

extern s32 D_801B2B70;
extern s32 D_801B2B74;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80095498(void)
{
    D_801B2B70 = 1;
    D_801B2B74 = 1;
}
