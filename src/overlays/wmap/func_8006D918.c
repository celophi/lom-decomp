#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern void *D_8011CF24;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B24A0;
extern s32 D_801B2410;
extern s32 D_801B2414;

/** @brief Compose the effect transform, draw it, and advance its rotation and countdown. */
void func_8006D918(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_80182DC0);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_801B24A0, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    func_8006CD98(D_8011CF24, 0, 36, 183, 0x7A40, 1, -1);
    PopMatrix();
    remaining = D_801B2414 - 1;
    D_801B24A0.vz = (u16)(D_801B24A0.vz + 320);
    D_801B2414 = remaining;
    if (remaining == 0)
    {
        D_801B2410++;
    }
}
