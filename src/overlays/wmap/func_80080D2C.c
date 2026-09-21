#include "common.h"

extern void func_8006CFE4(s32, s32, s32, s32, s32, s32);
extern u8 D_800DA448[];
extern s32 *D_80139280;
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B27D8;
extern s32 D_801B27DC;

/** @brief Draw the active effect and advance when the countdown expires. */
void func_80080D2C(void)
{
    s32 remaining_ticks;

    D_80139280[5] = -1;
    if (D_801B24B4 != 0)
    {
        func_8006CFE4((s32)D_800DA448, (s32)D_80139CC8, 0xC, D_801B24B4, D_801B24B4, 0);
    }
    remaining_ticks = D_801B27DC - 1;
    D_801B27DC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B27D8 += 1;
    }
}
