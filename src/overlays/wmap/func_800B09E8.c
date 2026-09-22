#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80139280;
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_801B2F60;
extern s32 D_801B2F64;

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void func_800B09E8(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x1E, 1, 0xFF, 4, 6, D_80139280 + 0x50);
    if (--D_801B2F64 == 0)
    {
        D_801B2F60 += 1;
    }
}
