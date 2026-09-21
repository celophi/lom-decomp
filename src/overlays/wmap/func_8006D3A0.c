#include "common.h"

extern s32 D_801B10A8;
extern s32 D_801B10AC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006D3A0(void)
{
    D_801B10A8 = 1;
    D_801B10AC = 1;
}
