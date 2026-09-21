#include "common.h"

extern s32 D_801B2AE0;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2AE4;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800923FC(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x13, 0x14, 0);
    if (--D_801B2AE4 == 0)
    {
        D_801B2AE0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80092478(void)
{
    D_801B2AE0 += 1;
}
