#include "common.h"

extern void func_800A32A4(void);
extern s32* D_80139280;
extern s32 D_801B2DCC;
extern s32 D_801B2DC8;
extern void func_8006AEE0(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A325C(void)
{
    D_801B2DCC = 0x20;
    D_80139280[35] = -1;
    D_801B2DC8 += 1;
    func_800A32A4();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A32A4(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0xA, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2DCC == 0)
    {
        D_801B2DC8 += 1;
    }
}
