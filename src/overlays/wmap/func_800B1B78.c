#include "common.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B2498[];
extern s32* D_8011CF1C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2FC8;
extern s32 D_801B2FCC;

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1B78(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_801B2498);
    func_800675F0(D_8011CF1C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1, D_80182DE4, 0, -0xA, -1);
    value = D_80182DE4 - 0x10;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x8;
    timer = D_801B2FCC;
    ((u16*)D_801B2498)[2] += 0x4;
    next_timer = timer - 1;
    D_801B2FCC = next_timer;
    if (next_timer == 0)
    {
        D_801B2FC8++;
    }
}
