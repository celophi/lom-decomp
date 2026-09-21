#include "common.h"

extern void func_8006CFE4(s32, s32, s32, s32, s32, s32);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B2698;
extern s32 D_801B269C;

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void func_8007A3D0(void)
{
    s32 value;
    s32 remaining_ticks;

    if (D_801B24B4 != 0)
    {
        func_8006CFE4((s32)D_800DA448, (s32)D_80139CC8, 0x18, D_801B24B4, D_801B24B4, 0);
        value = D_801B24B4 - 8;
        D_801B24B4 = value;
        if (value < 0)
        {
            D_801B24B4 = 0;
        }
    }
    remaining_ticks = D_801B269C - 1;
    D_801B269C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2698 += 1;
    }
}
