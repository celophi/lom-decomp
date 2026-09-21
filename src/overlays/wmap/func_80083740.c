#include "common.h"

extern s32 D_801B2848;
extern s32 D_801B284C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80083740(void)
{
    D_801B2848 = 1;
    D_801B284C = 1;
}
