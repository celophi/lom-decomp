#include "common.h"

extern s32 D_801B2BF8;
extern s32 D_801B2BFC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098604(void)
{
    D_801B2BF8 = 1;
    D_801B2BFC = 1;
}
