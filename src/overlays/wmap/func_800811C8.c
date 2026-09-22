#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern s32 D_801B2840;
extern s32 D_801B2844;

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void func_800811C8(void)
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
    func_8006AFAC(0x13, 0x45, 0x13, 0x20, 0xFD0, 4, 0x20, 1);
    remaining = D_801B2844 - 1;
    D_801B2844 = remaining;
    if (remaining == 0)
    {
        D_801B2840 += 1;
    }
}
