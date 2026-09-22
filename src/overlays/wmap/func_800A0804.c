/* Partial WMAP decompilation: 68.461540% (gcc280_g0). */
#include "common.h"

extern s32 D_80182D48;
extern s16 D_801398C8;
extern s32 D_801B2D48;

/**
 * @brief Reset the world-map cursor state and bump the transition counter.
 * @note Best match ~68.46% (gcc280_g0); residual is a sched2 lui-ordering tie
 *       (permuter territory).
 */
void func_800A0804(void)
{
    s32 *a;
    s16 *b;

    a = &D_80182D48;
    a[1] = 0;
    a[0] = 0;
    b = &D_801398C8;
    b[1] = 0;
    b[0] = 0;
    D_801B2D48 += 1;
}
