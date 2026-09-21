#include "common.h"

extern void func_80090E48(void);
extern s32* D_80139280;
extern s32 D_801B2A9C;
extern s32 D_801B2A98;
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80090E00(void)
{
    D_801B2A9C = 0x20;
    D_80139280[25] = -1;
    D_801B2A98 += 1;
    func_80090E48();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80090E48(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x14, 0, 0x7F, 0x1, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2A9C == 0)
    {
        D_801B2A98 += 1;
    }
}
