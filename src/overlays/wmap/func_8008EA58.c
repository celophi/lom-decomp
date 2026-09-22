#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80182DF0;
extern s32 D_801B2A60;
extern s32 D_801B2A64;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_8008EA58(void)
{
    s32 remaining;

    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E, 0x32, 1, 0x81, 2, 0);
    D_80182DF0 += 8;
    remaining = D_801B2A64 - 1;
    D_801B2A64 = remaining;
    if (remaining == 0)
    {
        D_801B2A60 += 1;
    }
}
