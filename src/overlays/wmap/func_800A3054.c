#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_800A309C(void);
extern s32* D_80139280;
extern s32 D_801B2DC4;
extern s32 D_801B2DC0;
extern u8 D_800DA448[];
extern u8 D_80139CC8[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A3054(void)
{
    D_801B2DC4 = 0x20;
    D_80139280[25] = -1;
    D_801B2DC0 += 1;
    func_800A309C();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A309C(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA448, D_80139CC8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2DC4 == 0)
    {
        D_801B2DC0 += 1;
    }
}
