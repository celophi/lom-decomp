#include "common.h"
#include "sdk/libgte.h"

extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B24F0;
extern s32 D_801B24F4;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);

/** @brief Draw and brighten two rotating layers, then advance their shared countdown. */
void func_800709E8(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_800DCF18, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 12);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_800DCF18, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz - 4);
    PopMatrix();
    intensity = D_801B2468 + 8;
    D_801B2468 = intensity;
    if (intensity >= 130)
    {
        D_801B2468 = 129;
    }
    remaining = D_801B24F4 - 1;
    D_801B24F4 = remaining;
    if (remaining == 0)
    {
        D_801B24F0++;
    }
}
