#include "common.h"
#include "sdk/libgte.h"

extern void *D_8011CF1C;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B2418;
extern s32 D_801B241C;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void func_8006DDEC(void)
{
    s32 remaining;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF1C, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF1C, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    remaining = D_801B241C - 1;
    D_801B241C = remaining;
    if (remaining == 0)
    {
        D_801B2418++;
    }
}
