#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80182DF4;
extern s32 D_801B2D40;
extern s32 D_801B2D44;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A0444(void)
{
    s32 remaining;

    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC, 0x8C, 1, 0x81, 1, 8, 1);
    D_80182DF4 += 8;
    remaining = D_801B2D44 - 1;
    D_801B2D44 = remaining;
    if (remaining == 0)
    {
        D_801B2D40 += 1;
    }
}
