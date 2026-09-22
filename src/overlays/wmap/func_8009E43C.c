#include "wmap_resource_support.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

/** @brief Set effect flags and color, play sound 41, and begin a 24-tick delay. */
void func_8009E43C(void)
{
    D_8013B208 = 1;
    func_8006683C(0x701040);
    D_801ADAF4 = 8;
    D_801ADAE0 = 1;
    func_800652A8(0x29, 0x80);
    D_801B2CDC = 0x18;
    D_801B2CD8 += 1;
}
