#include "common.h"

extern s32 D_801B26E8;
extern s32 D_801B26EC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007BB30(void)
{
    D_801B26E8 = 1;
    D_801B26EC = 1;
}
