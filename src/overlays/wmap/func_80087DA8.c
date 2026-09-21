#include "common.h"

extern s32 D_801B2920;
extern s32 D_801B2924;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80087DA8(void)
{
    D_801B2920 = 1;
    D_801B2924 = 1;
}
