#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2DD8;
extern s32 D_801B2DDC;
extern void func_8006683C(s32 arg0);

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void func_800A4218(void)
{
    D_80139244 = 0;
    func_8006683C(0x504060);
    D_801ADAF4 = 8;
    D_801B2DDC = 0x1E;
    D_801B2DD8 += 1;
}
