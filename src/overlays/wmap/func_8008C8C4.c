/* Partial WMAP decompilation: 87.588780% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);
extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern VECTOR D_80182DC0;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B4;
extern s32 D_801B2A30;
extern s32 D_801B2A34;

/** @brief Draw two oscillating effect layers and update their intensity. */
void func_8008C8C4(void)

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
    func_8006CD98(D_8011CF24, first_frame, 20, 0x35, 0x7800, 4096, D_801B2468);
    rotation->vz = (u16) (rotation->vz - 0x18);
    rotation = &D_801B2498;
    second_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_8011CF24, second_frame, 20, 0x35, 0x7800, 4096, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    intensity = D_801B2468 + 1;
    D_801B2468 = intensity;
    if (intensity >= 0x81)
    {
        D_801B2468 = 0x80;
    }
    remaining = D_801B2A34 - 1;
    D_801B2A34 = remaining;
    if (remaining == 0)
    {
        D_801B2A30 += 1;
    }
}
