#include "common.h"

extern void func_800851D4(void);
extern s32* D_80139280;
extern s32 D_801B28A4;
extern s32 D_801B28A0;
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008518C(void)
{
    D_801B28A4 = 0x20;
    D_80139280[15] = -1;
    D_801B28A0 += 1;
    func_800851D4();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800851D4(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x5, 0, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28A4 == 0)
    {
        D_801B28A0 += 1;
    }
}
