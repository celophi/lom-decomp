#include "common.h"

extern s32 D_80139280;
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_801B2F58;
extern s32 D_801B2F5C;
extern void func_8006A2FC(u8 *a0, u8 *a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void func_800B07CC(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xA, 1, 0xFF, 2, 6, D_80139280 + 0x28);
    if (--D_801B2F5C == 0)
    {
        D_801B2F58 += 1;
    }
}
