#include "common.h"

extern s32 D_801B26E0;
extern s32 D_801B26E4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007B988(void)
{
    D_801B26E0 = 1;
    D_801B26E4 = 1;
}
