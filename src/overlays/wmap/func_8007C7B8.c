#include "common.h"

extern s32 D_801B2710;
extern s32 D_801B2714;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007C7B8(void)
{
    D_801B2710 = 1;
    D_801B2714 = 1;
}
