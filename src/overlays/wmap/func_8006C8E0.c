#include "common.h"

extern s32 D_8011D4FC;
extern s32 D_8013B254;
extern s32 D_80182E34;
extern s32 D_801ADAEC;
extern s32 D_801B1098;
extern s32 D_801B109C;

/** @brief Set world-map flags and advance to a two-tick delay. */
void func_8006C8E0(void)
{
    if (D_8011D4FC != 0x1F)
    {
        D_8013B254 = 2;
    }
    D_80182E34 = 3;
    D_801ADAEC = 0;
    D_801B109C = 2;
    D_801B1098 += 1;
}
