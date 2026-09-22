#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_8009F584(void);
extern s32* D_80139280;
extern s32 D_801B2D0C;
extern s32 D_801B2D08;
extern u8 D_800D9790[];
extern u8 D_80139A78[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009F53C(void)
{
    D_801B2D0C = 0x40;
    D_80139280[15] = -1;
    D_801B2D08 += 1;
    func_8009F584();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F584(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9790, D_80139A78, 0x5A, 0xFF, 0x1, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2D0C == 0)
    {
        D_801B2D08 += 1;
    }
}
