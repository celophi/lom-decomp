#include "common.h"

extern s32 D_801B24D8;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;
extern s32 D_801B24DC;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071E00(void)
{
    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, 0xB, 0x2, 0);
    if (--D_801B24DC == 0)
    {
        D_801B24D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80071E7C(void)
{
    D_801B24D8 += 1;
}
