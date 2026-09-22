#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B25D8;
extern s32 D_801B2858;
extern s32 D_801B285C;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80083A10(void)
{
    s32 remaining;

    func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0);
    D_801B25D8 += 8;
    remaining = D_801B285C - 1;
    D_801B285C = remaining;
    if (remaining == 0)
    {
        D_801B2858 += 1;
    }
}
