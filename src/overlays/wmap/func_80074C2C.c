#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 99.365080% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"

extern u8 D_800DCF18[];
extern VECTOR D_80139888;
extern SVECTOR D_801B24A0;
extern s32 D_801B25D0;
extern s32 D_801B25D4;
extern s32 D_801B25D8;

/** @brief Draw the rotating effect, raise its intensity, and advance its countdown. */
void func_80074C2C(void)
{
    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    RotMatrix(&D_801B24A0, &matrix);
    TransMatrix(&matrix, &D_80139888);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    func_8006CD98(D_800DCF18, 0, 36, 183, 0x7A40, 1, -1);
    PopMatrix();
    intensity = D_801B25D8 + 15;
    D_801B25D8 = intensity;
    if (intensity >= 129)
    {
        D_801B25D8 = 128;
    }
    D_801B24A0.vz = (u16)(D_801B24A0.vz + 40);
    remaining = D_801B25D4 - 1;
    D_801B25D4 = remaining;
    D_80139888.vz -= 7000;
    if (remaining == 0)
    {
        D_801B25D0++;
    }
}
