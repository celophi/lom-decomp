#include "common.h"
#include "sdk/libgte.h"

extern u8 D_800DCF18[];
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern SVECTOR D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B2ED0;
extern s32 D_801B2ED4;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Approach the effect depth, draw its fading layer, and advance the countdown. */
void func_800AB444(void)
{
    MATRIX transform;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 3500;
    D_801B2650.vz = depth;
    if (depth < 10000)
    {
        D_801B2650.vz = 10000;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &transform);
    TransMatrix(&transform, &D_8011CF60);
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);
    if (D_80182DE8 != 0)
    {
        func_800675F0(D_800DCF18, 0, 4, 53, 0x7800, 1, D_80182DE8, 50, -20, -1);
        intensity = D_80182DE8 - 4;
        D_80182DE8 = intensity;
        if (intensity < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B2ED4 - 1;
    D_801B2ED4 = remaining;
    if (remaining == 0)
    {
        D_801B2ED0++;
    }
}
