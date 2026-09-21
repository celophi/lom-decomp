#include "common.h"

extern s32 D_801B2460;
extern s32 D_801B2464;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800705A0(void)
{
    D_801B2460 = 1;
    D_801B2464 = 1;
}
