#include "common.h"

extern void func_8009CFC8(void);
extern s32* D_80139280;
extern s32 D_801B2CB4;
extern s32 D_801B2CB0;
extern void func_8006AEE0(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009CF80(void)
{
    D_801B2CB4 = 0x80;
    D_80139280[15] = -1;
    D_801B2CB0 += 1;
    func_8009CFC8();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CFC8(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9CB8, D_80139B68, 0x18, 0xFF, 0x1, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2CB4 == 0)
    {
        D_801B2CB0 += 1;
    }
}
