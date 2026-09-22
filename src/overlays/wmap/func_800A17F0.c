#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;

/** @brief World-map step handler: kick two jobs and advance the step. */
void func_800A17F0(void)
{
    D_8013B208 = 1;
    func_8006683C(0x601040);
    D_801ADAF4 = 8;
    func_800652A8(0x2C, 0x80);
    D_801B2D5C = 0xF;
    D_801B2D58 += 1;
}
