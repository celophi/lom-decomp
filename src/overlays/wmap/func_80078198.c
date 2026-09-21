#include "common.h"

extern s32 D_801B2618;
extern s32 D_801B261C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80078198(void)
{
    D_801B2618 = 1;
    D_801B261C = 1;
}
