#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80073048(void);
extern void func_800737F4(void);
extern s32 D_801B2518;

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void func_800737B8(void)
{
    func_8006CAC0(func_80073048);
    D_801B2518 += 1;
    func_800737F4();
}
