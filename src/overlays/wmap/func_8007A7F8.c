#include "common.h"

extern s32 D_801B26B0;
extern s32 D_801B26B4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007A7F8(void)
{
    D_801B26B0 = 1;
    D_801B26B4 = 1;
}
