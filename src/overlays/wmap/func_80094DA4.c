#include "common.h"

extern s32 D_801B2B58;
extern s32 D_801B2B5C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80094DA4(void)
{
    D_801B2B58 = 1;
    D_801B2B5C = 1;
}
