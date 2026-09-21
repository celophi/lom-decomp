#include "common.h"

extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2430;
extern s32 D_801B2434;
extern void func_8006E6B8(void);

/** @brief World-map step tick: bump a counter, run the sub-step, and expire the timer. */
void func_8006F678(void)
{
    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006E6B8();
    if (--D_801B2434 == 0)
    {
        D_801B2430 += 1;
    }
}
