#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern SVECTOR D_8013B240;
extern s32 D_801B2B08;
extern s32 D_801B2B0C;
extern s32 D_80182DF4;
extern s32 D_80139234;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Draw the expanding effect and advance its rotation and countdown. */
void func_80091850(void)
{
    s32 scale;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B240);
    func_800675F0(D_800DCF18, 0, 10, 183, 0x7A40, 0x1001, D_80182DF4, 0, 5, D_80139234 / 16);
    scale = D_80139234 - 128;
    D_80139234 = scale;
    intensity = D_80182DF4 + 2;
    D_80182DF4 = intensity;
    if (intensity >= 130)
    {
        D_80182DF4 = 129;
    }
    if (scale < 16)
    {
        D_80139234 = 16;
    }
    PopMatrix();
    remaining = D_801B2B0C - 1;
    D_8013B240.vz = (u16)(D_8013B240.vz + 220);
    D_801B2B0C = remaining;
    if (remaining == 0)
    {
        D_801B2B08++;
    }
}
