#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_800886A4(void);
extern s32* D_80139280;
extern s32 D_801B294C;
extern s32 D_801B2948;
extern u8 D_800DA7B8[];
extern u8 D_80139D68[];

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008865C(void)
{
    D_801B294C = 0x20;
    D_80139280[25] = -1;
    D_801B2948 += 1;
    func_800886A4();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800886A4(void)
{
    func_8006A2FC(D_800DA7B8, D_80139D68, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B294C == 0)
    {
        D_801B2948 += 1;
    }
}
