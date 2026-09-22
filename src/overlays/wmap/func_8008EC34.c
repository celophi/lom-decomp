#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2A64;
extern s32 D_801B2A60;

/** @brief World-map step handler: spawn a sprite via a long argument list, then countdown-advance. */
void func_8008EC34(void)
{
    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0);
    if (--D_801B2A64 == 0)
    {
        D_801B2A60 += 1;
    }
}
