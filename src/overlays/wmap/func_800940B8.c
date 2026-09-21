#include "common.h"

extern s32 D_801B2B30;
extern s32 D_801B2B34;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800940B8(void)
{
    D_801B2B30 = 1;
    D_801B2B34 = 1;
}
