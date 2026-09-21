#include "common.h"

extern u8 D_800D93F4[];
extern s32 D_801399D0;
extern s32 D_8011CF4C;
extern s32 D_80182DF0;
extern s32 D_801B25C0;
extern s32 D_801B25C4;
extern void func_8006CC4C(void *dst, void *src);
extern void func_80066F9C(void *a, s32 b, s32 c, s32 d, s32 e);

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void func_80076790(void)
{
    func_8006CC4C(D_800D93F4, &D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x10, 0xB, 0);
    *(s16 *)(D_800D93F4 + 0x24) = (u16)D_80182DF0;
    *(s16 *)(D_800D93F4 + 0x22) = (u16)D_80182DF0;
    D_80182DF0 -= 0x10;
    if (D_80182DF0 < 0)
    {
        D_80182DF0 = 0;
    }
    if (--D_801B25C4 == 0)
    {
        D_801B25C0 += 1;
    }
}
