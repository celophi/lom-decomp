#include "common.h"

extern s32 D_801B26B8;
extern s32 D_801B26BC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007A980(void)
{
    D_801B26B8 = 1;
    D_801B26BC = 1;
}
