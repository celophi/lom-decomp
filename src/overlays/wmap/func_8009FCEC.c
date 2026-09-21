#include "common.h"

extern void func_8009FD34(void);
extern s32* D_80139280;
extern s32 D_801B2D2C;
extern s32 D_801B2D28;
extern void func_8006AEE0(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009FCEC(void)
{
    D_801B2D2C = 0x20;
    D_80139280[25] = -1;
    D_801B2D28 += 1;
    func_8009FD34();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009FD34(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2D2C == 0)
    {
        D_801B2D28 += 1;
    }
}
