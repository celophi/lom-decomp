#include "common.h"

extern s32 D_801B2EC8;
extern s32 D_801B2ECC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD2B4(void)
{
    D_801B2EC8 = 1;
    D_801B2ECC = 1;
}
