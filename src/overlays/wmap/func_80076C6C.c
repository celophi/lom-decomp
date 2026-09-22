#include "common.h"
#include "sdk/libgte.h"

extern u8 D_800DCF18[];
extern s32 D_8011CF74;
extern s32 D_801B2470;
extern SVECTOR D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B25F8;
extern s32 D_801B25FC;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Approach the effect depth and draw its alternating-brightness fade. */
void func_80076C6C(void)
{
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 10000;
    D_801B2650.vz = depth;
    if (depth < 1000)
    {
        D_801B2650.vz = 1000;
    }
    PushMatrix();
    func_8006CFA8(&D_801B2650, &D_801B24A0);
    if (D_801B2470 != 0)
    {
        if (D_8011CF74 & 1)
        {
            func_800675F0(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470, 5, -20, -1);
        }
        else
        {
            func_800675F0(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470 / 2, 5, -20, -1);
        }
        intensity = D_801B2470 - 2;
        D_801B2470 = intensity;
        if (intensity < 0)
        {
            D_801B2470 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B25FC - 1;
    D_801B25FC = remaining;
    if (remaining == 0)
    {
        D_801B25F8++;
    }
}
