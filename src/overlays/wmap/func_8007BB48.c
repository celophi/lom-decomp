#include "wmap_view_effects.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B26E8;
extern s32 D_801B26EC;

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void func_8007BB48(void)
{
    D_8013B208 = 1;
    func_8006683C(0x503030);
    D_801ADAF4 = 4;
    D_801B26EC = 0x10;
    D_801B26E8 += 1;
}
