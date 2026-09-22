#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800983E8(void);
extern s32 D_801B2BE8;
extern s32 D_801B2BEC;

/** @brief World-map step handler: register a callback and advance the step counter. */
void func_80097F10(void)
{
    func_800652A8(0x27, 0x80);
    func_8006CAC0(func_800983E8);
    D_801B2BEC = 0x10;
    D_801B2BE8 += 1;
}
