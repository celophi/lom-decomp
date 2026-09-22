#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2E28;
extern s32 D_801B2E2C;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5758(void)
{
    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    if (--D_801B2E2C == 0)
    {
        D_801B2E28 += 1;
    }
}
