#include "common.h"

extern s32 D_801B2570;
extern s32 D_801B2574;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800751E0(void)
{
    D_801B2570 = 1;
    D_801B2574 = 1;
}
