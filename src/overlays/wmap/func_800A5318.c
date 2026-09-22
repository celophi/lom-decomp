#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B25D8;
extern s32 D_801B2E20;
extern s32 D_801B2E24;

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void func_800A5318(void)
{
    s32 c;

    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32,
                  0x64, 0xB4, 0x81, 0x81, 8, 1);
    D_801B25D8 += 8;
    c = D_801B2E24 - 1;
    D_801B2E24 = c;
    if (c == 0)
    {
        D_801B2E20 += 1;
    }
}
