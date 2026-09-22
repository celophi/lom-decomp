#include "common.h"

extern void (*D_800D59D0[])(void);
extern u32 D_801B28D0;
extern s32 D_801B28D4;

/** @brief Reset or dispatch the current effect phase.
 * @param reset Nonzero to restart the effect.
 * @return One while active, otherwise zero.
 */
s32 func_80086354(s32 reset)
{
    s32 active;
    if (reset != 0)
    {
        D_801B28D0 = 1;
        D_801B28D4 = 1;
        return 1;
    }
    if (D_801B28D0 < 6U)
    {
        D_800D59D0[D_801B28D0]();
        active = 1;
    }
    else
    {
        active = 0;
    }
    return active;
}
