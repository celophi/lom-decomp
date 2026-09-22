#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;
extern void func_800A1F00(void);
extern void func_8006683C(s32 color);

/** @brief World-map step: register a callback, set flags, advance the step. */
void func_800A1970(void)
{
    func_8006CAC0(func_800A1F00);
    D_80139244 = 1;
    func_8006683C(0x351040);
    D_801ADAF4 = 3;
    D_801B2D5C = 2;
    D_801B2D58 += 1;
}
