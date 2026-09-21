#include "common.h"
#include "sdk/libgte.h"

extern u8 *D_8011CF24;
extern s32 D_80182DE4;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
/** @brief Translation whose depth read is retained by the original draw routine. */
typedef struct
{
    s32 vx;
    s32 vy;
    volatile s32 vz;
    s32 pad;
} WmapEffectTranslation;

extern WmapEffectTranslation D_80139888;
extern SVECTOR D_8013B240;
extern s32 D_801B2460;
extern s32 D_801B2464;
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);

/** @brief Compose the effect transform, draw and brighten it, and advance its countdown. */
void func_8006ECC0(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;
    s32 intensity;

    /* Preserve the original depth read before installing the transform. */
    (void)D_80139888.vz;
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, (VECTOR *)&D_80139888);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_8013B240, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_80182DE4 != 0)
    {
        func_8006CD98(D_8011CF24 + 0x6000, 0, 4, -1, -1, 1, D_80182DE4);
    }
    PopMatrix();
    intensity = D_80182DE4 + 10;
    D_80182DE4 = intensity;
    if (intensity >= 256)
    {
        D_80182DE4 = 255;
    }
    remaining = D_801B2464 - 1;
    D_801B2464 = remaining;
    if (remaining == 0)
    {
        D_801B2460++;
    }
}
