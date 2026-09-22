#include "wmap_view_effects.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAF4;
extern s32 D_801B28B8;
extern s32 D_801B28BC;
extern void func_800868E0(void);
extern void func_8008600C(void);

/** @brief Register world-map callbacks, seed a mode value, and advance step counters. */
void func_80085DA0(void)
{
    func_8006CAC0(func_800868E0);
    func_8006683C(0x801045);
    D_801ADAF4 = 4;
    func_8006CAC0(func_8008600C);
    D_801B28BC = 2;
    D_801B28B8 += 1;
}
