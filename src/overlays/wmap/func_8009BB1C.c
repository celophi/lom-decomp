#include "wmap_resource_support.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801B2C6C;
extern s32 D_801B2C68;

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void func_8009BB1C(void)
{
    D_8013B208 = 1;
    func_800652A8(0x2E, 0x80);
    D_801B2C6C = 4;
    D_801B2C68 += 1;
}
