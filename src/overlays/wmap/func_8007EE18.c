#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 86.951920% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"

extern s8 D_80051B4C[];
extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B4;
extern s32 D_801B2790;
extern s32 D_801B2794;

/** @brief Draw two oscillating effect layers and update their intensity. */
void func_8007EE18(void)

{
    s32 first_frame;
    SVECTOR *rotation;
    s32 second_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    rotation = &D_801B2490;
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_800DCF18, first_frame, 4, 0x35, 0x7800, 0, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x14);
    rotation = &D_801B2498;
    second_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_800DCF18, second_frame, 4, 0x35, 0x7800, 0, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 8) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 4) & 0xFF;
    intensity = D_801B2468 - 2;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = D_801B2794 - 1;
    D_801B2794 = remaining;
    if (remaining == 0)
    {
        D_801B2790 += 1;
    }
}
