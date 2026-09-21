#include "common.h"

extern s8 *func_80099754(s32 arg0);
extern u8 D_801AFBD0[];
extern s32 D_8011CF74;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;

/**
 * @brief World-map actor tick: advance timers, bump a wave index, and expire the step.
 */
void func_80099918(void)
{
    s8 *obj;
    u8 *base;

    obj = func_80099754(1);
    base = D_801AFBD0;
    if (*(s16 *)(base + 0xE) < 100)
    {
        *(s16 *)(base + 0xE) += 1;
    }
    if (*(s32 *)(base + 0x8) < 0x1E0)
    {
        *(s32 *)(base + 0x8) += 4;
    }
    if ((D_8011CF74 & 3) == 0)
    {
        if (obj[6] < 0xF)
        {
            obj[6] += 1;
        }
    }
    if (--D_801B2C50 == 0)
    {
        D_801B2C4C += 1;
    }
}
