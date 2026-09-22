#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80182DF0;
extern s32 D_801B2B60;
extern s32 D_801B2B64;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80094F10(void)
{
    s32 remaining;

    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2B64 - 1;
    D_801B2B64 = remaining;
    if (remaining == 0)
    {
        D_801B2B60 += 1;
    }
}
