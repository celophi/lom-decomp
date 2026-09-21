#include "common.h"

extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32* D_80139280;
extern s32 D_801B2DB8;
extern s32 D_801B2DBC;

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void func_800A2E9C(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x2E, 0x1, 0xFF, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2DBC == 0)
    {
        D_801B2DB8 += 1;
    }
}
