#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern u8 *D_8011CF24;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern s32 D_80182DC8;
extern s32 D_801B2428;
extern s32 D_801B242C;
extern s32 D_801B2470;
extern s32 D_801B2474;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;

/** @brief Advance the effect depth, fade its intensity, and update the countdown. */
void func_8006E544(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;
    s32 peak;

    depth = D_801B2478.vz - 0x960;
    D_801B2478.vz = depth;
    if (depth < 0x3E8)
    {
        D_801B2478.vz = (s32) D_80182DC8;
        D_801B2474 = D_801B2470;
    }
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_801B2478);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_801B24A8, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_801B2474 != 0)
    {
        func_8006CD98(D_8011CF24 + 0x5000, 0, 4, -1, -1, 1, D_801B2474);
    }
    PopMatrix();
    peak = D_801B2470 - 1;
    intensity = D_801B2474 - 9;
    D_801B2470 = peak;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    if (peak < 0)
    {
        D_801B2470 = 0;
    }
    remaining = D_801B242C - 1;
    D_801B242C = remaining;
    if (remaining == 0)
    {
        D_801B2428 += 1;
    }
}
