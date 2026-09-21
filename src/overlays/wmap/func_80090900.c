#include "common.h"

extern s32 D_801B2A88;
extern s32 D_801B2A8C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090900(void)
{
    D_801B2A88 = 1;
    D_801B2A8C = 1;
}
