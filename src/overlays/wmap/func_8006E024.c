#include "common.h"
#include "sdk/libgte.h"

extern u8 *D_8011CF24;
extern void func_8006CD98(void *, s32, s32, s32, s32, s32, s32);
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern VECTOR D_80139870;
extern s32 D_80139878;
extern SVECTOR D_8013B238;
extern s32 D_801B2420;
extern s32 D_801B2424;
extern s32 D_801B2488;

/** @brief Draw the effect at successive depths and advance its countdown. */
void func_8006E024(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    switch (D_801B2488)
   
  {
    case 0:
        D_80139878 = 8000;
        break;
    case 1:
        D_80139878 = 18000;
        break;
    case 2:
        D_80139878 = 10000;
        break;
    }
    D_801B2488 += 1;
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_80139870);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_8013B238, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    func_8006CD98(D_8011CF24 + 0x4000, 0, 4, -1, -1, 1, -1);
    PopMatrix();
    remaining = D_801B2424 - 1;
    D_801B2424 = remaining;
    if (remaining == 0)
    {
        D_801B2420 += 1;
    }
}
