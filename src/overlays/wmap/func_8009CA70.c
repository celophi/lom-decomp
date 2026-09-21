#include "common.h"

extern s32 D_801B2CA0;
extern s32 D_801B2CA4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009CA70(void)
{
    D_801B2CA0 = 1;
    D_801B2CA4 = 1;
}
