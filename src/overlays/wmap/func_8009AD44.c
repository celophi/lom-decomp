#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_8009A3E0(void);
extern void func_8009AD8C(void);
extern s32 D_800D9164;
extern s32 D_801B2C54;

/** @brief World-map step handler: install a callback, advance the step counter, chain to the next step. */
void func_8009AD44(void)
{
    D_800D9164 = 1;
    func_8006CAC0(func_8009A3E0);
    D_801B2C54 += 1;
    func_8009AD8C();
}
