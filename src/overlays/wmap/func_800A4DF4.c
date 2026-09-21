#include "common.h"

extern void func_8006B328(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_80182DE4;
extern s32 D_801B2E10;
extern s32 D_801B2E14;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A4DF4(void)
{
    s32 remaining;

    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32, 0x64, 1, 0x7F, 0x7F, 0, 0);
    D_80182DE4 += 8;
    remaining = D_801B2E14 - 1;
    D_801B2E14 = remaining;
    if (remaining == 0)
    {
        D_801B2E10 += 1;
    }
}
