#include "wmap_effect_primitives.h"
/* Partial WMAP decompilation: 99.488370% (gcc280_g0). */
#include "common.h"

extern s32 D_801B2B60;
extern s32 D_801B2B64;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800950EC(void)
{
    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 1, 0x7F, 4, 0);
    if (--D_801B2B64 == 0)
    {
        D_801B2B60 += 1;
    }
}
