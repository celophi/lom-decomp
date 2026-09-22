#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B2498[];
extern s32* D_8011CF28;
extern s32 D_80182DE4;
extern s32 D_801B2F2C;
extern s32 D_801B2F28;

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void func_800ADEB0(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_801B2498);
    func_800675F0(D_8011CF28, 0, 0x7, 0x35, 0x7840, 0x1001, D_80182DE4, 0, 0x32, -1);
    value = D_80182DE4 - 2;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    timer = D_801B2F2C;
    ((u16*)D_801B2498)[2] += 0x8;
    next_timer = timer - 1;
    D_801B2F2C = next_timer;
    if (next_timer == 0)
    {
        D_801B2F28++;
    }
}
