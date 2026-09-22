#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B24A0[];
extern s32* D_8011CF24;
extern s32 D_80139240;
extern s32 D_80182DE8;
extern s32 D_801B2FD0;
extern s32 D_801B2FD4;

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1D7C(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_801B24A0);
    func_800675F0(D_8011CF24, (D_80139240 / 0x10) & 3, 0xA, 0x35, 0x7840, 0x1, D_80182DE8, 0, -0xA, -1);
    value = D_80182DE8 - 0x10;
    D_80182DE8 = value;
    if (value < 0)
    {
        D_80182DE8 = 0;
    }
    D_80139240 += 0x10;
    timer = D_801B2FD4;
    ((u16*)D_801B24A0)[2] += 0x20;
    next_timer = timer - 1;
    D_801B2FD4 = next_timer;
    if (next_timer == 0)
    {
        D_801B2FD0++;
    }
}
