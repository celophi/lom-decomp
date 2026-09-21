#include "common.h"

#include "sdk/libgte.h"

extern SVECTOR D_801398C8;
extern s32 D_801B2448;
extern s32 D_801B244C;
extern u16 D_801B248C;

/** @brief Adjust the Z angle and advance when the countdown reaches zero. */
void func_800701DC(void)
{
    s32 remaining_ticks;
    D_801398C8.vz = (u16)(D_801398C8.vz + D_801B248C);
    remaining_ticks = D_801B244C - 1;
    D_801B244C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2448 += 1;
    }
}
