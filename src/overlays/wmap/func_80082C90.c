#include "common.h"

extern void func_80082CDC(void);
extern s16 D_800D939C[];
extern s32 D_801B2824;
extern s32 D_801B2820;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80082C90(void)
{
    D_800D939C[19] = 2;
    D_800D939C[17] = 0;
    D_801B2824 = 0x8;
    D_801B2820 += 1;
    func_80082CDC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082CDC(void)
{
    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, 0x8, 0xA, 0);
    if (--D_801B2824 == 0)
    {
        D_801B2820 += 1;
    }
}
