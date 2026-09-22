#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B25D8;
extern s32 D_801B2778;
extern s32 D_801B277C;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_8007E9C4(void)
{
    s32 remaining;

    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA, 0x1F4, 0x32, 0x81, 0x81, 8, 0);
    D_801B25D8 += 8;
    remaining = D_801B277C - 1;
    D_801B277C = remaining;
    if (remaining == 0)
    {
        D_801B2778 += 1;
    }
}
