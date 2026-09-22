#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_8011CF2C;
extern s32 D_80139280;
extern s32 D_801B2FB8;
extern s32 D_801B2FBC;
extern void func_8008ECF8(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern void func_8006534C(s32 arg0, s32 arg1);

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void func_800B2E38(void)
{
    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    if (--D_801B2FBC == 0)
    {
        D_801B2FB8 += 1;
    }
}
