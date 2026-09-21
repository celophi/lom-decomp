#include "common.h"

extern s32 D_801B26F0;
extern s32 D_801B26F4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007BFF4(void)
{
    D_801B26F0 = 1;
    D_801B26F4 = 1;
}
