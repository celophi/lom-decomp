#include "common.h"

extern s32 D_801B2878;
extern s32 D_801B287C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80084914(void)
{
    D_801B2878 = 1;
    D_801B287C = 1;
}
