#include "common.h"

extern s32 D_801B2788;
extern s32 D_801B278C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007F5B8(void)
{
    D_801B2788 = 1;
    D_801B278C = 1;
}
