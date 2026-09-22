#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800652A8(s32 a0, s32 a1);
extern void func_80071CEC(void);
extern s32 D_801B24C0;
extern s32 D_801B24C4;

/** @brief World-map step handler: register a callback and advance the step counter. */
void func_80071580(void)
{
    func_800652A8(0x24, 0x80);
    func_8006CAC0(func_80071CEC);
    D_801B24C4 = 4;
    D_801B24C0 += 1;
}
