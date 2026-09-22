#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_8009D1D0(void);
extern s32* D_80139280;
extern s32 D_801B2CBC;
extern s32 D_801B2CB8;
extern u8 D_800DA398[];
extern u8 D_80139CA8[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009D188(void)
{
    D_801B2CBC = 0x20;
    D_80139280[25] = -1;
    D_801B2CB8 += 1;
    func_8009D1D0();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D1D0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA398, D_80139CA8, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2CBC == 0)
    {
        D_801B2CB8 += 1;
    }
}
