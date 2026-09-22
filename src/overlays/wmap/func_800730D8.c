#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32 a0);
extern void func_80073430(void);
extern s32 D_8013B208;
extern s32 D_801B2514;
extern s32 D_801B2510;

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void func_800730D8(void)
{
    D_8013B208 = 1;
    func_8006683C(0x404040);
    func_8006CAC0(func_80073430);
    func_800652A8(0xF, 0x80);
    D_801B2514 = 8;
    D_801B2510 += 1;
}
