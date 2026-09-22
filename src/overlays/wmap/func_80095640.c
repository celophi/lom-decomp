#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_80095688(void);
extern s32* D_80139280;
extern s32 D_801B2B7C;
extern s32 D_801B2B78;
extern u8 D_800DB578[];
extern u8 D_80139FE8[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80095640(void)
{
    D_801B2B7C = 0x40;
    D_80139280[15] = -1;
    D_801B2B78 += 1;
    func_80095688();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80095688(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2B7C == 0)
    {
        D_801B2B78 += 1;
    }
}
