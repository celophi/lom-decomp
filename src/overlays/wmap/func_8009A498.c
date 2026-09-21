#include "common.h"

extern s32 D_801B2C4C;
extern s32 D_801B2C50;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009A498(void)
{
    D_801B2C4C = 1;
    D_801B2C50 = 1;
}
