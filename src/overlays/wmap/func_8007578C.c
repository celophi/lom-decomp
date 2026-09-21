#include "common.h"

extern void *D_801399AC;
extern s32 D_8011D538;
extern u8 D_800D9268[];
extern s32 D_801B24B4;
extern s32 D_801B2578;
extern s32 D_801B257C;
extern void func_800757FC(void);

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void func_8007578C(void)
{
    u8 *base;

    D_801399AC = &D_8011D538;
    base = D_800D9268;
    *(u8 *)(base + 0xB6) = 0xF;
    *(s16 *)(base + 0xB2) = 0;
    *(s16 *)(base + 0xBE) = 0;
    *(s16 *)(base + 0xC0) = -1;
    D_801B24B4 = 0;
    D_801B257C = 8;
    D_801B2578 += 1;
    func_800757FC();
}
