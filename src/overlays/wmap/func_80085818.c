#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern void *D_8011CF24;
extern void *D_8011CF28;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_80182DF0;
extern s32 D_801B28F8;
extern s32 D_801B28FC;

/** @brief Draw and brighten two rotating effect layers and advance their shared countdown. */
void func_80085818(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF24, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2490.vz = (u16)(D_801B2490.vz + 24);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF28, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2498.vz = (u16)(D_801B2498.vz - 16);
    PopMatrix();
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 130)
    {
        D_80182DF0 = 129;
    }
    remaining = D_801B28FC - 1;
    D_801B28FC = remaining;
    if (remaining == 0)
    {
        D_801B28F8++;
    }
}
