#include "common.h"
#include "sdk/libgte.h"

extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 D_801B2A30;
extern s32 D_801B2A34;

/** @brief Animate and fade two counter-rotating effect layers. */
void func_8008CA70(void)
{
    s32 first_frame;
    
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x18);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    intensity = D_801B2468 - 4;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = D_801B2A34 - 1;
    D_801B2A34 = remaining;
    if (remaining == 0)
    {
        D_801B2A30 += 1;
    }
}
