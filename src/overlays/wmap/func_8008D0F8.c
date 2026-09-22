#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void *D_8011CF30;
extern VECTOR D_801B2660;
extern SVECTOR D_801B2678;
extern s32 D_801B25DC;
extern s32 D_801B2A48;
extern s32 D_801B2A4C;

/** @brief Draw the fading effect and advance its countdown. */
void func_8008D0F8(void)
{
    s32 intensity;
    s32 remaining;

    if (D_801B2660.vz < 0x7530)
    {
        D_801B2660.vz = 0x7530;
    }
    PushMatrix();
    func_8006CFA8(&D_801B2660, &D_801B2678);
    if (D_801B25DC != 0)
    {
        func_8006CD98(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
        intensity = D_801B25DC - 1;
        D_801B25DC = intensity;
        if (intensity < 0)
        {
            D_801B25DC = 0;
        }
        D_801B2678.vz = (u16) (D_801B2678.vz + 4);
    }
    PopMatrix();
    remaining = D_801B2A4C - 1;
    D_801B2A4C = remaining;
    if (remaining == 0)
    {
        D_801B2A48 += 1;
    }
}
