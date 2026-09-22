#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_800996B0(void);
extern s32* D_80139280;
extern s32 D_801B2C44;
extern s32 D_801B2C40;
extern u8 D_800DB578[];
extern u8 D_80139FE8[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80099668(void)
{
    D_801B2C44 = 0x20;
    D_80139280[35] = -1;
    D_801B2C40 += 1;
    func_800996B0();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800996B0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}
