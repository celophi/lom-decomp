#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B2498[];
extern s32* D_8011CF2C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2DA0;
extern s32 D_801B2DA4;

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800A0E84(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_801B2498);
    func_800675F0(D_8011CF2C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1001, D_80182DE4, 0, 0x14, -1);
    value = D_80182DE4 - 2;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x10;
    timer = D_801B2DA4;
    ((u16*)D_801B2498)[2] += 0x38;
    next_timer = timer - 1;
    D_801B2DA4 = next_timer;
    if (next_timer == 0)
    {
        D_801B2DA0++;
    }
}
