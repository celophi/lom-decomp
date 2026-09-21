#include "common.h"

extern s32 D_801B2610;
extern s32 D_801B2614;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80078048(void)
{
    D_801B2610 = 1;
    D_801B2614 = 1;
}
