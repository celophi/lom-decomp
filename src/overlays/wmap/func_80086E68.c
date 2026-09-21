#include "common.h"

extern void func_80086EB4(void);
extern s16 D_800D93C8[];
extern s32 D_801B2904;
extern s32 D_801B2900;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80086E68(void)
{
    D_800D93C8[19] = 16;
    D_800D93C8[17] = 0;
    D_801B2904 = 0x8;
    D_801B2900 += 1;
    func_80086EB4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086EB4(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x18, 0x9, 0);
    if (--D_801B2904 == 0)
    {
        D_801B2900 += 1;
    }
}
