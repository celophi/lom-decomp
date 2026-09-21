#include "common.h"

extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2730;
extern s32 D_801B2734;
extern void func_8007B70C(void);

/** @brief World-map step tick: bump a counter, run the sub-step, and expire the timer. */
void func_8007D054(void)
{
    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8007B70C();
    if (--D_801B2734 == 0)
    {
        D_801B2730 += 1;
    }
}
