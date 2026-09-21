#include "common.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B240[];
extern s32* D_8011CF2C;
extern s32 D_80182DF4;
extern s32 D_801B2F34;
extern s32 D_801B2F30;

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void func_800ADF80(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_8013B240);
    func_800675F0(D_8011CF2C, 0, 0x7, 0x35, 0x7840, 0x1, D_80182DF4, 0, 0x1E, -1);
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    timer = D_801B2F34;
    ((u16*)D_8013B240)[2] -= 0x8;
    next_timer = timer - 1;
    D_801B2F34 = next_timer;
    if (next_timer == 0)
    {
        D_801B2F30++;
    }
}
