#include "common.h"

#include "sdk/libgte.h"

extern SVECTOR D_801398C8;
extern s32 D_801B2448;
extern s32 D_801B244C;
extern s32 D_801B248C;

/** @brief Reduce angular speed toward zero, update the Z angle, and count down the step. */
void func_80070260(void)
{
    s32 speed;
    s32 remaining_ticks;
    speed = D_801B248C - 5;
    D_801B248C = speed;
    if (speed < 0)
    {
        D_801B248C = 0;
    }
    D_801398C8.vz = (u16)(D_801398C8.vz + (u16)D_801B248C);
    remaining_ticks = D_801B244C - 1;
    D_801B244C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2448 += 1;
    }
}
