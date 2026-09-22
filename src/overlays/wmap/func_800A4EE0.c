#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2E10;
extern s32 D_801B2E14;

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_800A4EE0(void)
{
    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--D_801B2E14 == 0)
    {
        D_801B2E10 += 1;
    }
}
