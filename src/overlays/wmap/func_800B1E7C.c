#include "common.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B24A8[];
extern s32* D_8011CF28;
extern s32 D_8013924C;
extern s32 D_80182DEC;
extern s32 D_801B2FD8;
extern s32 D_801B2FDC;

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1E7C(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_801B24A8);
    func_800675F0(D_8011CF28, (D_8013924C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1, D_80182DEC, 0, -0xA, -1);
    D_8013924C += 0x10;
    value = D_80182DEC + 0x8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = D_801B2FDC;
    ((u16*)D_801B24A8)[2] += 0x8;
    next_timer = timer - 1;
    D_801B2FDC = next_timer;
    if (next_timer == 0)
    {
        D_801B2FD8++;
    }
}
