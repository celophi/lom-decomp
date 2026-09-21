#include "common.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B240[];
extern s32* D_8011CF24;
extern s32 D_80182DF4;
extern s32 D_801B2E0C;
extern s32 D_801B2E08;

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void func_800A36F4(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_8013B240);
    func_800675F0(D_8011CF24, 0, 0xA, 0x36, 0x78C0, 0x1001, D_80182DF4, 0, 0xA, -1);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    timer = D_801B2E0C;
    ((u16*)D_8013B240)[2] += 0x16;
    next_timer = timer - 1;
    D_801B2E0C = next_timer;
    if (next_timer == 0)
    {
        D_801B2E08++;
    }
}
