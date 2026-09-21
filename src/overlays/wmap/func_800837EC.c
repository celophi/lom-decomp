#include "common.h"

extern void func_8006AFAC(s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2848;
extern s32 D_801B284C;

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_800837EC(void)
{
    s32 remaining_ticks;

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006AFAC(0x54, 0x5C, 0x13, 0x20, 0x1000, 0x20, 8, 2);
    remaining_ticks = D_801B284C - 1;
    D_801B284C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2848 += 1;
    }
}
