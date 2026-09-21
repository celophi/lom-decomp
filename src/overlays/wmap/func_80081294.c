#include "common.h"

extern void func_8006AFAC(s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern s32 D_801B2848;
extern s32 D_801B284C;

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void func_80081294(void)
{
    s32 value;
    s32 remaining;

    value = D_80139980 - 1;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    if (!(D_8011CF74 & 3))
    {
        D_801B0FD0 -= 1;
    }
    if (D_801B0FD0 < 0)
    {
        D_801B0FD0 = 0;
    }
    func_8006AFAC(0x54, 0x5C, 0x13, 0x20, 0x1000, 0x20, 8, 2);
    remaining = D_801B284C - 1;
    D_801B284C = remaining;
    if (remaining == 0)
    {
        D_801B2848 += 1;
    }
}
