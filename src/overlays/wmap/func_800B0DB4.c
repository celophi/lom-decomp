#include "common.h"

extern void func_8006AEE0(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800DAFA0[];
extern u8 D_80139ED8[];
extern s32* D_80139280;
extern s32 D_801B2F74;
extern s32 D_801B2F70;

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800B0DB4(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAFA0, D_80139ED8, 0x18, 0xFF, 0x1, 0x4, 0, (s32)D_80139280);
    if (--D_801B2F74 == 0)
    {
        D_801B2F70 += 1;
    }
}
