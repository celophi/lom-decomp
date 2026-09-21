#include "common.h"
#include "sdk/libgte.h"

extern u8 D_800DCF18[];
extern VECTOR D_801B2478;
extern VECTOR D_8011CF60;
extern SVECTOR D_801B24A8;
extern s32 D_801B2538;
extern s32 D_801B253C;
extern s32 D_801B2470;
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void func_80072A58(void)
{
    MATRIX matrix;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 2400;
    D_801B2478.vz = depth;
    if (depth < 10)
    {
        D_801B2478.vz = 10;
    }
    PushMatrix();
    RotMatrix(&D_801B24A8, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_801B2470 != 0)
    {
        func_8006CD98(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470);
    }
    PopMatrix();
    intensity = D_801B2470 - 9;
    D_801B2470 = intensity;
    if (intensity < 0)
    {
        D_801B2470 = 0;
    }
    remaining = D_801B253C - 1;
    D_801B253C = remaining;
    if (remaining == 0)
    {
        D_801B2538++;
    }
}
