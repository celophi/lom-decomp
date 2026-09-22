#include "common.h"

extern void func_800773B8(s32, s32, s32);
extern s32 D_80182DF4;
extern s32 D_801B2648;
extern s32 D_801B264C;

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void func_80078DF0(void)
{
    s32 value;
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    value = D_80182DF4 - 8;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    remaining_ticks = D_801B264C - 1;
    D_801B264C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2648 += 1;
    }
}
