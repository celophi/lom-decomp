#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern void *D_8011CF24;
extern VECTOR D_80139870;
extern VECTOR D_8011CF60;
extern SVECTOR D_8013B238;
extern s32 D_801B2608;
extern s32 D_801B260C;
extern s32 D_801B24B4;

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void func_80076EB4(void)
{
    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    if (D_80139870.vz < 10)
    {
        D_80139870.vz = 10;
    }
    PushMatrix();
    RotMatrix(&D_8013B238, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_801B24B4 != 0)
    {
        func_8006CD98(D_8011CF24, 0, 4, -1, -1, 1, D_801B24B4);
        intensity = D_801B24B4 - 8;
        D_801B24B4 = intensity;
        if (intensity < 0)
        {
            D_801B24B4 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B260C - 1;
    D_801B260C = remaining;
    if (remaining == 0)
    {
        D_801B2608++;
    }
}
