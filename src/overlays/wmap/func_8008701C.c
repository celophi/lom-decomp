#include "common.h"
#include "sdk/libgte.h"

extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern s32 D_8011CF1C;
extern s32 D_80182DF0;
extern s32 D_801B2928;
extern s32 D_801B292C;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_8006CD98(s32, s32, s32, s32, s32, s32, s32);

/** @brief Draw the rotating effect, reduce its scale, and update the sequence timer. */
void func_8008701C(void)
{
    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DF0);
    D_801B2490.vz += 100;
    PopMatrix();
    D_80182DF0 -= 8;
    if (D_80182DF0 < 0)
    {
        D_80182DF0 = 0;
    }
    if (--D_801B292C == 0)
    {
        D_801B2928++;
    }
}
