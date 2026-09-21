#include "common.h"

extern s32 D_801B26C0;
extern s32 D_801B26C4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007AAD0(void)
{
    D_801B26C0 = 1;
    D_801B26C4 = 1;
}
