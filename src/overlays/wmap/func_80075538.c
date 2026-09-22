#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2570;
extern s32 D_801B2574;
extern void func_80075DD8(void);

/** @brief World-map step handler: register a callback and advance the step. */
void func_80075538(void)
{
    func_8006CAC0(func_80075DD8);
    D_801B2574 = 1;
    D_801B2570 += 1;
}
