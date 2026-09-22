#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern u8 D_800DCF18[];
extern VECTOR D_80139888;
extern SVECTOR D_801B24A0;
extern s32 D_801B25D0;
extern s32 D_801B25D4;
extern s32 D_801B25D8;

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void func_80074D28(void)
{
    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    if (D_801B25D8 != 0)
    {
        PushMatrix();
        RotMatrix(&D_801B24A0, &matrix);
        TransMatrix(&matrix, &D_80139888);
        SetRotMatrix(&matrix);
        SetTransMatrix(&matrix);
        func_8006CD98(D_800DCF18, 0, 36, 183, 0x7A40, 1, D_801B25D8);
        intensity = D_801B25D8 - 4;
        D_801B25D8 = intensity;
        if (intensity < 0)
        {
            D_801B25D8 = 0;
        }
        PopMatrix();
        D_801B24A0.vz = (u16)(D_801B24A0.vz + 40);
    }
    remaining = D_801B25D4 - 1;
    D_801B25D4 = remaining;
    if (remaining == 0)
    {
        D_801B25D0++;
    }
}
