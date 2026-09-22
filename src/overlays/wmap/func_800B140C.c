#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2F84;
extern s32 D_801B2F80;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800B140C(void)
{
    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1);
    if (--D_801B2F84 == 0)
    {
        D_801B2F80 += 1;
    }
}
