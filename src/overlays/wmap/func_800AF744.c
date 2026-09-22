#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800AF790(void);
extern s16 D_800D9318[];
extern s32 D_801B2F04;
extern s32 D_801B2F00;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AF6C8(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x10, 0x5, 0);
    if (--D_801B2F04 == 0)
    {
        D_801B2F00 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800AF744(void)
{
    D_800D9318[19] = 4;
    D_800D9318[17] = 0;
    D_801B2F04 = 0x20;
    D_801B2F00 += 1;
    func_800AF790();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AF790(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x10, 0x5, 0);
    if (--D_801B2F04 == 0)
    {
        D_801B2F00 += 1;
    }
}
