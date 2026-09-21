#include "common.h"

extern s32 D_801B2AB8;
extern s32 D_801B2ABC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800913AC(void)
{
    D_801B2AB8 = 1;
    D_801B2ABC = 1;
}
