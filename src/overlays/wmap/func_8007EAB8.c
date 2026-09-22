#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2778;
extern s32 D_801B277C;

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_8007EAB8(void)
{
    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA,
                  0x1F4, 0x32, 0x81, 0x81, 8, 0);
    if (--D_801B277C == 0)
    {
        D_801B2778 += 1;
    }
}
