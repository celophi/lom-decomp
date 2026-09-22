#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern s32 D_801B27E0;
extern s32 D_801B27E4;

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void func_8007F2D0(void)
{
    s32 value;
    s32 remaining;

    value = D_80139980 - 2;
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
    func_8006AFAC(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining = D_801B27E4 - 1;
    D_801B27E4 = remaining;
    if (remaining == 0)
    {
        D_801B27E0 += 1;
    }
}
