#include "common.h"

extern s32 D_80139280;
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_801B2F60;
extern s32 D_801B2F64;
extern void func_8006A2FC(u8 *a0, u8 *a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void func_800B09E8(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x1E, 1, 0xFF, 4, 6, D_80139280 + 0x50);
    if (--D_801B2F64 == 0)
    {
        D_801B2F60 += 1;
    }
}
