#include "common.h"

extern void func_800773B8(s32, s32, s32);
extern s32 D_80182DF4;
extern s32 D_801B2648;
extern s32 D_801B264C;

/** @brief Draw the effect, raise its value to at most 129, and update the countdown. */
void func_80078CB4(void)
{
    s32 value;
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    remaining_ticks = D_801B264C - 1;
    D_801B264C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2648 += 1;
    }
}
