#include "wmap_view_effects.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAF4;
extern s32 D_801B2A70;
extern s32 D_801B2A74;
extern void func_80091088(void);
extern void func_80090478(void);

/** @brief Register world-map callbacks, seed a mode value, and advance step counters. */
void func_800900BC(void)
{
    func_8006CAC0(func_80091088);
    func_8006683C(0x602030);
    D_801ADAF4 = 4;
    func_8006CAC0(func_80090478);
    D_801B2A74 = 8;
    D_801B2A70 += 1;
}
