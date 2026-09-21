#include "common.h"

extern s32 D_801B2B48;
extern s32 D_801B2B4C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80094AF8(void)
{
    D_801B2B48 = 1;
    D_801B2B4C = 1;
}
