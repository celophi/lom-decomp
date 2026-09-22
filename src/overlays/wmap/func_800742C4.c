#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80182DEC;
extern s32 D_801B2558;
extern s32 D_801B255C;

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void func_800742C4(void)
{
    s32 value;
    s32 remaining_ticks;

    func_8006CFE4((s32)D_800DB578, (s32)D_80139FE8, 0x18, 0x81, 0x81, 0x10);
    value = D_80182DEC - 0xA;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    remaining_ticks = D_801B255C - 1;
    D_801B255C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2558 += 1;
    }
}
