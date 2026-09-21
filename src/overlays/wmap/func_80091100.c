#include "common.h"

extern s32 D_801B2AA8;
extern s32 D_801B2AAC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80091100(void)
{
    D_801B2AA8 = 1;
    D_801B2AAC = 1;
}
