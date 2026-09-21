#include "common.h"

extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B27D8;
extern s32 D_801B27DC;
extern void func_8006CFE4(void *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/** @brief World-map step handler: spawn a sub-object and expire the step counter. */
void func_80080C88(void)
{
    func_8006CFE4(D_800DA448, D_80139CC8, 0xC, D_801B24B4, D_801B24B4, 0);
    if (--D_801B27DC == 0)
    {
        D_801B27D8 += 1;
    }
}
