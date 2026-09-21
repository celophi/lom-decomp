#include "common.h"

extern void func_8006B328(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B25DC;
extern s32 D_801B2E28;
extern s32 D_801B2E2C;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A5660(void)
{
    s32 remaining;

    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    D_801B25DC += 8;
    remaining = D_801B2E2C - 1;
    D_801B2E2C = remaining;
    if (remaining == 0)
    {
        D_801B2E28 += 1;
    }
}
