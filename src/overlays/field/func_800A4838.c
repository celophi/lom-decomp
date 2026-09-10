#include "common.h"

extern s32 D_8011F338;
extern s32 D_8011F350;
extern s32 D_8011F354;
extern s32 D_8011F3A8;
extern s32 D_8011F3AC;
extern s32 D_8011F3B4;
extern s32 D_8011F3BC;
extern s32 D_8011F3C0;

/** @brief Advance interpolation and the three-phase field animation counter. */
void func_800A4838(void)
{
    if (D_8011F338 != 0)
    {
        D_8011F350 += (D_8011F354 - D_8011F350) / D_8011F338;
        D_8011F338 -= 1;
    }
    else
    {
        D_8011F350 = D_8011F354;
    }

    if (D_8011F3A8 < 8)
    {
        D_8011F3C0 += 0x200;
    }
    else if (D_8011F3A8 < 0x10)
    {
        D_8011F3C0 += 0x100;
    }
    else if (D_8011F3A8 < 0x20)
    {
        D_8011F3C0 += 0x80;
    }
    else
    {
        D_8011F3B4 = 0;
        D_8011F3AC = 1;
        D_8011F3A8 = 0;
        D_8011F3BC = 0x80;
        return;
    }
    D_8011F3BC += 4;
    D_8011F3A8 += 1;
}
