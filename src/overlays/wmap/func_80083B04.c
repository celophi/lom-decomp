#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B285C;
extern s32 D_801B2858;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80083B04(void)
{
    func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0);
    if (--D_801B285C == 0)
    {
        D_801B2858 += 1;
    }
}
