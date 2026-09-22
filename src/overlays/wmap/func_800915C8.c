#include "wmap_resource_support.h"
#include "common.h"

extern void func_8008ECF8(s32, s32, s32, s32);
extern s32 D_8011CF28;
extern s32 D_80139280;
extern s32 D_801B2AC8;
extern s32 D_801B2ACC;

/** @brief Draw the effect, select texture page 37, and update the sequence countdown. */
void func_800915C8(void)
{
    s32 value;

    func_8008ECF8(0x64, 0x96, D_8011CF28, D_80139280 + 0x78);
    func_8006534C(0x25, 2);
    value = D_801B2ACC - 1;
    D_801B2ACC = value;
    if (value == 0)
    {
        D_801B2AC8 += 1;
    }
}
