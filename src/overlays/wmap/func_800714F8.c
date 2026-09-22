#include "wmap_view_effects.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B24C0;
extern s32 D_801B24C4;

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void func_800714F8(void)
{
    D_8013B208 = 1;
    func_8006683C(0x701040);
    D_801ADAF4 = 4;
    D_801B24C4 = 0x14;
    D_801B24C0 += 1;
}
