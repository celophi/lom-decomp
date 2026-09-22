#include "common.h"
#include "sdk/libgte.h"

extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B2F1C;
extern s32 D_801B2F18;

/** @brief World-map step: spin the model matrix, draw the highlight, then tick the sub-counter. */
void func_800ADBB4(void)
{
    MATRIX m;

    D_801B2478.vz -= 0xDAC;
    if (D_801B2478.vz < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (D_80182DEC != 0)
    {
        func_800675F0(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, 0, 0, -1);
        D_80182DEC -= 2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    if (--D_801B2F1C == 0)
    {
        D_801B2F18 += 1;
    }
}
