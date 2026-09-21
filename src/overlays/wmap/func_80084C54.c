#include "common.h"

extern s32 D_801B2888;
extern s32 D_801B288C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80084C54(void)
{
    D_801B2888 = 1;
    D_801B288C = 1;
}
