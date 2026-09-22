#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_800884AC(void);
extern s32* D_80139280;
extern s32 D_801B2944;
extern s32 D_801B2940;
extern u8 D_800D9F20[];
extern u8 D_80139BD8[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80088464(void)
{
    D_801B2944 = 0x20;
    D_80139280[15] = -1;
    D_801B2940 += 1;
    func_800884AC();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800884AC(void)
{
    func_8006A2FC(D_800D9F20, D_80139BD8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2944 == 0)
    {
        D_801B2940 += 1;
    }
}
