#include "wmap_view_effects.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

/** @brief Set world-map flags and color, then begin a 48-tick delay. */
void func_8009E800(void)
{
    D_80139244 = 1;
    D_801ADAF4 = 1;
    func_8006683C(0x201010);
    D_801B2CDC = 0x30;
    D_801B2CD8 += 1;
}
