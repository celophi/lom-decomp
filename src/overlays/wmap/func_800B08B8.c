#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80139280;
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_801B2F58;
extern s32 D_801B2F5C;

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void func_800B08B8(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xA, 1, 0xFF, 2, 7, D_80139280 + 0x28);
    if (--D_801B2F5C == 0)
    {
        D_801B2F58 += 1;
    }
}
