#include "common.h"
#include "sdk/libgte.h"

extern void *D_8011CF28;
extern s32 D_80139264;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2670;
extern s32 D_801B2DA8;
extern s32 D_801B2DAC;
extern s32 D_801B25D8;
extern s32 D_80139234;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void func_800A10B4(void)
{
    s32 intensity;
    s32 remaining;

    if (D_801B25D8 != 0)
    {
        PushMatrix();
        func_8006CFA8(&D_80182DC0, &D_801B2670);
        func_800675F0(D_8011CF28, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
        intensity = D_801B25D8 - 8;
        D_801B25D8 = intensity;
        D_80139264++;
        if (intensity < 0)
        {
            D_801B25D8 = 0;
        }
        PopMatrix();
        D_801B2670.vz = (u16)(D_801B2670.vz + 30);
    }
    remaining = D_801B2DAC - 1;
    D_801B2DAC = remaining;
    if (remaining == 0)
    {
        D_801B2DA8++;
    }
}
