#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80182DF0;
extern s32 D_801B2B10;
extern s32 D_801B2B14;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80092D38(void)
{
    s32 remaining;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0, 0x140, 0x32, 1, 0x81, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2B14 - 1;
    D_801B2B14 = remaining;
    if (remaining == 0)
    {
        D_801B2B10 += 1;
    }
}
