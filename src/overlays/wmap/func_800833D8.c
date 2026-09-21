#include "common.h"

extern void func_80083424(void);
extern s16 D_800D9420[];
extern s32 D_801B283C;
extern s32 D_801B2838;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399D8[];
extern s32 D_8011CF4C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008335C(void)
{
    func_8006CC4C(D_800D9420, D_801399D8);
    func_80066F9C(D_800D9420, D_8011CF4C, 0x8, 0xA, 0);
    if (--D_801B283C == 0)
    {
        D_801B2838 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800833D8(void)
{
    D_800D9420[19] = 8;
    D_800D9420[17] = 0;
    D_801B283C = 0x8;
    D_801B2838 += 1;
    func_80083424();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80083424(void)
{
    func_8006CC4C(D_800D9420, D_801399D8);
    func_80066F9C(D_800D9420, D_8011CF4C, 0x8, 0xA, 0);
    if (--D_801B283C == 0)
    {
        D_801B2838 += 1;
    }
}
