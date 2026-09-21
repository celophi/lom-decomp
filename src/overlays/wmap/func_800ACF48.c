#include "common.h"

extern s32 D_801B2EB0;
extern s32 D_801B2EB4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACF48(void)
{
    D_801B2EB0 = 1;
    D_801B2EB4 = 1;
}
