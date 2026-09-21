#include "common.h"

extern s32 D_801B2CD0;
extern s32 D_801B2CD4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009E2C8(void)
{
    D_801B2CD0 = 1;
    D_801B2CD4 = 1;
}
