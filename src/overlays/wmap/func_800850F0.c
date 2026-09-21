#include "common.h"

extern s32 D_801B28A0;
extern s32 D_801B28A4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800850F0(void)
{
    D_801B28A0 = 1;
    D_801B28A4 = 1;
}
