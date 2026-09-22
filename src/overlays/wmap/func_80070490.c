#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80182DF4;
extern s32 D_801B2458;
extern s32 D_801B245C;

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void func_80070490(void)
{
    s32 c;

    if (D_80182DF4 > 0)
    {
        func_8006CFE4(D_800DB578, D_80139FE8, 0x28, 0, D_80182DF4, 3);
    }
    D_80182DF4 -= 4;
    c = D_801B245C - 1;
    D_801B245C = c;
    if (c == 0)
    {
        D_801B2458 += 1;
    }
}
