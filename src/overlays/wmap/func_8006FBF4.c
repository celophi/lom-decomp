#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2400;
extern s32 D_801B2404;

/** @brief Set world-map flags and color, then begin an eight-tick delay. */
void func_8006FBF4(void)
{
    D_80139244 = 1;
    D_801ADAF4 = 8;
    func_8006683C(0x262726);
    D_801B2404 = 8;
    D_801B2400 += 1;
}
