#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B25D8;
extern s32 D_801B2EE8;
extern s32 D_801B2EEC;

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800AD7F8(void)
{
    s32 remaining;

    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    D_801B25D8 += 8;
    remaining = D_801B2EEC - 1;
    D_801B2EEC = remaining;
    if (remaining == 0)
    {
        D_801B2EE8 += 1;
    }
}
