#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_8013B208;
extern s32 D_801B2570;
extern s32 D_801B2574;

/** @brief Enable the world-map flag, set the drawing color, and start a two-tick delay. */
void func_800751F8(void)
{
    D_8013B208 = 1;
    func_8006683C(0x404040);
    D_801B2574 = 2;
    D_801B2570 += 1;
}
