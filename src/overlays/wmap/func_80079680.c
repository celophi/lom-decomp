#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);
extern void *D_8011CF1C;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern s32 D_80182DF4;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;
extern s32 D_801B26C0;
extern s32 D_801B26C4;

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void func_80079680(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 0x7D0;
    D_801B2478.vz = depth;
    if (depth < 0x1F40)
    {
        D_801B2478.vz = 0x1F40;
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
    if (D_80182DF4 != 0)
    {
        func_8006CD98(D_8011CF1C, 0, 0xC, 0x35, 0x7800, 1, D_80182DF4);
        intensity = D_80182DF4 - 6;
        D_80182DF4 = intensity;
        if (intensity < 0)
        {
            D_80182DF4 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B26C4 - 1;
    D_801B26C4 = remaining;
    if (remaining == 0)
    {
        D_801B26C0 += 1;
    }
}
