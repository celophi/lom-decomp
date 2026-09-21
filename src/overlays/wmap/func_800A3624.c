#include "common.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B238[];
extern s32* D_8011CF24;
extern s32 D_80182DF0;
extern s32 D_801B2E04;
extern s32 D_801B2E00;

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void func_800A3624(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_8013B238);
    func_800675F0(D_8011CF24, 0, 0xA, 0x36, 0x7880, 0x1001, D_80182DF0, 0, 0xA, -1);
    value = D_80182DF0 - 2;
    D_80182DF0 = value;
    if (value < 0)
    {
        D_80182DF0 = 0;
    }
    timer = D_801B2E04;
    ((u16*)D_8013B238)[2] += 0x16;
    next_timer = timer - 1;
    D_801B2E04 = next_timer;
    if (next_timer == 0)
    {
        D_801B2E00++;
    }
}
