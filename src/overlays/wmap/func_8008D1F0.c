#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);
extern u8 D_800DCF18[];
extern VECTOR D_8011CF60;
extern VECTOR D_801B2650;
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern s32 D_801B2A50;
extern s32 D_801B2A54;

/** @brief Draw the fading effect and advance its countdown. */
void func_8008D1F0(void)
{
    MATRIX transform;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = depth;
    if (depth < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &transform);
    TransMatrix(&transform, &D_8011CF60);
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);
    if (D_80182DE8 != 0)
    {
        func_8006CD98(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8);
        intensity = D_80182DE8 - 2;
        D_80182DE8 = intensity;
        if (intensity < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B2A54 - 1;
    D_801B2A54 = remaining;
    if (remaining == 0)
    {
        D_801B2A50 += 1;
    }
}
