#include "wmap_view_effects.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void func_8009EB48(void)
{
    D_80139244 = 0;
    func_8006683C(0x602050);
    D_801ADAF4 = 7;
    D_801B2CDC = 0x75;
    D_801B2CD8 += 1;
}
