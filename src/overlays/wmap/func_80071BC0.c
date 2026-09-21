#include "common.h"

extern s32 D_801B24D0;
extern s32 D_801B24D4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80071BC0(void)
{
    D_801B24D0 = 1;
    D_801B24D4 = 1;
}
