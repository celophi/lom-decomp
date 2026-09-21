#include "common.h"

extern s32 D_801B2B20;
extern s32 D_801B2B24;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80093254(void)
{
    D_801B2B20 = 1;
    D_801B2B24 = 1;
}
