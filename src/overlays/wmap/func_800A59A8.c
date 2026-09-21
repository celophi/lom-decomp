#include "common.h"

extern void func_8006B328(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B25D8;
extern s32 D_801B2E30;
extern s32 D_801B2E34;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A59A8(void)
{
    s32 remaining;

    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    D_801B25D8 += 8;
    remaining = D_801B2E34 - 1;
    D_801B2E34 = remaining;
    if (remaining == 0)
    {
        D_801B2E30 += 1;
    }
}
