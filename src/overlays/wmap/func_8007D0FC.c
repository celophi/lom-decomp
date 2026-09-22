#include "common.h"

extern s32 D_80139980;
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2730;
extern s32 D_801B2734;
extern void func_8007B70C(void);

/**
 * @brief World-map step tick: age two timers with zero clamps, run the sub-step,
 *        and expire the step counter.
 */
void func_8007D0FC(void)
{
    D_80139980 -= 1;
    if (D_80139980 < 0)
    {
        D_80139980 = 0;
    }
    if ((D_8011CF74 & 3) == 0)
    {
        D_801B0FD0 -= 1;
    }
    if (D_801B0FD0 < 0)
    {
        D_801B0FD0 = 0;
    }
    func_8007B70C();
    if (--D_801B2734 == 0)
    {
        D_801B2730 += 1;
    }
}
