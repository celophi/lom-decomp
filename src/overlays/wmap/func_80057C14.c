#include "common.h"

extern void func_80057274(s32);
extern void func_800574D0(void);
extern void func_8005784C(s32);
extern void func_80057D2C(void);
extern s32 D_800DCF04;
extern s32 D_8011CF74;
extern s32 D_80139218;
extern s32 D_8013986C;
extern s32 D_8013B268;
extern s32 D_801ADAEC;

/** @brief Update map rendering and approach the requested fade intensity. */
void func_80057C14(void)
{
    if (D_8013B268 != 0)
    {
        switch (D_8013986C)
        {
        case 0:
            func_80057274(-1);
            func_800574D0();
            func_80057D2C();
            break;
        case 1:
            if (D_800DCF04 != 0)
            {
                func_80057274(D_800DCF04 - 1);
                func_8005784C(D_800DCF04 - 1);
            }
            break;
        }
    }
    if (D_8011CF74 & 1)
    {
        D_80139218 = (D_80139218 + 1) & 7;
    }
    if (D_8013B268 > D_801ADAEC)
    {
        D_8013B268 -= 8;
    }
    if (D_8013B268 < D_801ADAEC)
    {
        D_8013B268 += 8;
    }
    if (D_8013B268 >= 0x81)
    {
        D_801ADAEC = 0x80;
        D_8013B268 = 0x80;
    }
}
