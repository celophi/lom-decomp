#include "common.h"

extern void func_8007CEA4(void);
extern s16 D_800D93F4[];
extern s32 D_801B272C;
extern s32 D_801B2728;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007CE58(void)
{
    D_800D93F4[19] = 8;
    D_800D93F4[17] = 0;
    D_801B272C = 0x10;
    D_801B2728 += 1;
    func_8007CEA4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007CEA4(void)
{
    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x15, 0xB, 0);
    if (--D_801B272C == 0)
    {
        D_801B2728 += 1;
    }
}
