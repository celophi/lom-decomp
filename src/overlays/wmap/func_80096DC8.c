#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80182DF0;
extern s32 D_801B2BC0;
extern s32 D_801B2BC4;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80096DC8(void)
{
    s32 remaining;

    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2BC4 - 1;
    D_801B2BC4 = remaining;
    if (remaining == 0)
    {
        D_801B2BC0 += 1;
    }
}
