#include "common.h"

extern s32 D_801B2538;
extern s32 D_801B253C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80073B4C(void)
{
    D_801B2538 = 1;
    D_801B253C = 1;
}
