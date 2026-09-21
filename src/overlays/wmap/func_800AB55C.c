#include "common.h"
#include "sdk/libgte.h"

extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32* D_8011CF1C;
extern s32 D_80182DEC;
extern s32 D_801B2ED8;
extern s32 D_801B2EDC;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800AB55C(void)
{
    MATRIX m;
    s32 x;
    s32 timer;

    x = D_801B2478[2] - 0xDAC;
    D_801B2478[2] = x;
    if (x < 0x2710)
    {
        D_801B2478[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DEC != 0)
    {
        func_800675F0(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, -0x19, -0x32, -1);
        D_80182DEC -= 8;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    timer = D_801B2EDC - 1;
    D_801B2EDC = timer;
    if (timer == 0)
    {
        D_801B2ED8 += 1;
    }
}
