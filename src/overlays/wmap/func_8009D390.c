#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_8009D3D8(void);
extern s32* D_80139280;
extern s32 D_801B2CC4;
extern s32 D_801B2CC0;
extern u8 D_800DB158[];
extern u8 D_80139F28[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009D390(void)
{
    D_801B2CC4 = 0x20;
    D_80139280[45] = -1;
    D_801B2CC0 += 1;
    func_8009D3D8();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D3D8(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0xA0));
    if (--D_801B2CC4 == 0)
    {
        D_801B2CC0 += 1;
    }
}
