#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2B10;
extern s32 D_801B2B14;

/** @brief World-map effect spawn: submit a request and tick the refcount. */
void func_80092F1C(void)
{
    s32 c;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0,
                  0x140, 0x32, 1, 0x81, 4, 0);
    c = D_801B2B14 - 1;
    D_801B2B14 = c;
    if (c == 0)
    {
        D_801B2B10 += 1;
    }
}
