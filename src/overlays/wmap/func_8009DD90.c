#include "common.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B24A8[];
extern s32* D_8011CF28;
extern s32 D_8013923C;
extern s32 D_80182DEC;
extern s32 D_801B2D18;
extern s32 D_801B2D1C;

/**
 * @brief World-map step handler: render the animated actor, fade it out, scroll the
 *        shadow field, then advance when the frame counter expires.
 */
void func_8009DD90(void)
{
    s32 value;
    s32 timer;

    func_8006CFA8(D_80182DC0, D_801B24A8);
    func_800675F0(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001, D_80182DEC, 0, 0xA, -1);
    value = D_80182DEC - 8;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    timer = D_801B2D1C - 1;
    *(u16*)((u8*)D_801B24A8 + 4) += 0x38;
    D_801B2D1C = timer;
    D_8013923C += 1;
    if (timer == 0)
    {
        D_801B2D18 += 1;
    }
}
