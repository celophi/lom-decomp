#include "wmap_resource_support.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801B2EFC;
extern s32 D_801B2EF8;

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void func_800AED00(void)
{
    D_8013B208 = 1;
    func_800652A8(0x2F, 0x80);
    D_801B2EFC = 2;
    D_801B2EF8 += 1;
}
