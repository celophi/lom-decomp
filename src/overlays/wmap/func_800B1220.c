#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B25E0;
extern s32 D_801B2F80;
extern s32 D_801B2F84;

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void func_800B1220(void)
{
    s32 c;

    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78,
                  0xF0, 0x64, 0x81, 0x81, 4, 1);
    D_801B25E0 += 8;
    c = D_801B2F84 - 1;
    D_801B2F84 = c;
    if (c == 0)
    {
        D_801B2F80 += 1;
    }
}
