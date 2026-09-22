#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2B90;
extern s32 D_801B2B94;
extern void func_800969E8(void);

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void func_800963F0(void)
{
    func_8006CAC0(&func_800969E8);
    D_80139244 = 0;
    D_801ADAF4 = 0x10;
    func_8006683C(0x808080);
    D_801B2B94 = 2;
    D_801B2B90 += 1;
}
