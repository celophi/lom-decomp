#include "wmap_view_effects.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2B30;
extern s32 D_801B2B34;

/** @brief Set the drawing color and world-map values, then start a two-tick delay. */
void func_800944AC(void)
{
    D_801ADAF4 = 8;
    func_8006683C(0x562056);
    D_80139244 = 0;
    D_801B2B34 = 2;
    D_801B2B30 += 1;
}
