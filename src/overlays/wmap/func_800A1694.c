#include "common.h"

extern s32 D_80139978;
extern s16 D_8011CF4C[];
extern void func_8006CAC0(void (*fn)(void));
extern void func_800A1760(void);
extern s32 D_8013B20C;
extern s32 D_801B2D50;
extern void func_800A16FC(void);

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void func_800A1694(void)
{
    D_80139978 = 0x17;
    D_8011CF4C[0] = 0xA4;
    D_8011CF4C[1] = 0x69;
    func_8006CAC0(func_800A1760);
    D_8013B20C = 1;
    D_801B2D50 += 1;
    func_800A16FC();
}
