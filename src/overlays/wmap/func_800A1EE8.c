#include "common.h"

extern s32 D_801B2D60;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;
extern s32 D_801B2D64;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A1E6C(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x16, 0xB, 0);
    if (--D_801B2D64 == 0)
    {
        D_801B2D60 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A1EE8(void)
{
    D_801B2D60 += 1;
}
