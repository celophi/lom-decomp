#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_8009C954(void);
extern s32* D_80139280;
extern s32 D_801B2C9C;
extern s32 D_801B2C98;
extern u8 D_800DAB28[];
extern u8 D_80139E08[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009C90C(void)
{
    D_801B2C9C = 0x20;
    D_80139280[35] = -1;
    D_801B2C98 += 1;
    func_8009C954();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009C954(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C9C == 0)
    {
        D_801B2C98 += 1;
    }
}
