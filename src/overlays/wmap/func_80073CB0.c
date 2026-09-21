#include "common.h"

extern void func_80072D30(void);
extern s32 D_801B2540;
extern s32 D_801B2544;

/** @brief Update the sequence effect and advance when its countdown reaches zero. */
void func_80073CB0(void)
{
    s32 remaining_ticks;

    func_80072D30();
    remaining_ticks = D_801B2544 - 1;
    D_801B2544 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2540 += 1;
    }
}
