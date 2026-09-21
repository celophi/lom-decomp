#include "common.h"

extern s32 D_801B2968;
extern s32 D_801B296C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800897A4(void)
{
    D_801B2968 = 1;
    D_801B296C = 1;
}
