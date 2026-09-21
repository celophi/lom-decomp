#include "common.h"

extern void func_80097390(void);
extern s32* D_80139280;
extern s32 D_801B2BD4;
extern s32 D_801B2BD0;
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D9688[];
extern u8 D_80139A48[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80097348(void)
{
    D_801B2BD4 = 0x20;
    D_80139280[35] = -1;
    D_801B2BD0 += 1;
    func_80097390();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80097390(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2BD4 == 0)
    {
        D_801B2BD0 += 1;
    }
}
