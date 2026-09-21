#include "common.h"

extern void func_80072D30(void);
extern s32 D_80139980;
extern s32 D_801B2540;
extern s32 D_801B2544;

/** @brief Reduce the effect value toward zero, update it, and advance when the countdown expires. */
void func_80073D34(void)
{
    s32 value;
    s32 remaining_ticks;

    value = D_80139980 - 4;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    func_80072D30();
    remaining_ticks = D_801B2544 - 1;
    D_801B2544 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2540 += 1;
    }
}
