/* Partial WMAP decompilation: 96.750000% (gcc280_g0). */
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

/** @brief Draw and brighten the rotating effect while reducing its scale. */
void func_800A0F84(void)
{
    s32 scale;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2670);
    func_800675F0(D_8011CF28, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
    scale = D_80139234 - 32;
    D_80139264++;
    D_80139234 = scale;
    intensity = D_801B25D8 + 8;
    D_801B25D8 = intensity;
    if (intensity >= 98)
    {
        D_801B25D8 = 97;
    }
    if (scale < 16)
    {
        D_80139234 = 16;
    }
    PopMatrix();
    remaining = D_801B2DAC - 1;
    D_801B2670.vz = (u16)(D_801B2670.vz + 30);
    D_801B2DAC = remaining;
    if (remaining == 0)
    {
        D_801B2DA8++;
    }
}
