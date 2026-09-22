#include "wmap_resource_support.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801B2A14;
extern s32 D_801B2A10;

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void func_8008D6EC(void)
{
    D_8013B208 = 1;
    func_800652A8(0x25, 0x80);
    D_801B2A14 = 8;
    D_801B2A10 += 1;
}
