#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_80093140(void);
extern s32* D_80139280;
extern s32 D_801B2B1C;
extern s32 D_801B2B18;
extern u8 D_800D9688[];
extern u8 D_80139A48[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800930F8(void)
{
    D_801B2B1C = 0x20;
    D_80139280[35] = -1;
    D_801B2B18 += 1;
    func_80093140();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80093140(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x19, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2B1C == 0)
    {
        D_801B2B18 += 1;
    }
}
