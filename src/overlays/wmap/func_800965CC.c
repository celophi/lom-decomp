#include "common.h"

extern s32 D_801B2B98;
extern s32 D_801B2B9C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800965CC(void)
{
    D_801B2B98 = 1;
    D_801B2B9C = 1;
}
