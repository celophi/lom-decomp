#include "common.h"
#include "sdk/libgte.h"

extern void *D_8011CF1C;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B24B0;
extern s32 D_801B2418;
extern s32 D_801B241C;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void func_8006DC98(void)
{
    s32 remaining;
    s32 frame;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    frame = D_801B24B0 + 1;
    D_801B24B0 = frame;
    if (frame >= 8)
    {
        D_801B24B0 = 7;
    }
    if (D_801B2468 >= 5)
    {
        D_801B2468 -= 4;
    }
    func_8006CD98(D_8011CF1C, D_801B24B0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF1C, D_801B24B0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    remaining = D_801B241C - 1;
    D_801B241C = remaining;
    if (remaining == 0)
    {
        D_801B2418++;
    }
}
