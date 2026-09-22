#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2EEC;
extern s32 D_801B2EE8;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800AD8EC(void)
{
    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    if (--D_801B2EEC == 0)
    {
        D_801B2EE8 += 1;
    }
}
