/* Partial WMAP decompilation: 91.949150% (gcc280_g0). */
#include "common.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B238[];
extern s32* D_8011CF2C;
extern s32 D_80139234;
extern s32 D_801B25D8;
extern s32 D_801B2D10;
extern s32 D_801B2D14;

/**
 * @brief World-map step handler: render the animated actor, fade it out, scroll the
 *        shadow field, then advance when the frame counter expires.
 */
void func_8009DBB0(void)
{
    s32 value;
    s32 timer;


    func_8006CFA8(D_80182DC0, D_8013B238);
    func_800675F0(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 0, 0xF, -1);
    value = D_801B25D8 - 4;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    timer = D_801B2D14 - 1;
    *(u16*)((u8*)D_8013B238 + 4) += 0x14;
    D_801B2D14 = timer;
    D_80139234 += 1;
    if (timer == 0)
    {
        D_801B2D10 += 1;
    }
}
