#include "wmap_sequence_runtime.h"
#include "wmap_effect_backdrop.h"
#include "common.h"

extern s32 D_801398D0;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 D_80182D68;
extern s32 D_80139950[];
extern s32 D_80182D78;
extern s32 D_801B2E60;
extern void func_800A8864(void);

/** @brief World-map step: seed the scroll target from the current cell, then advance. */
void func_800A703C(void)
{
    func_8006D8F0(1);
    func_8006D870(1);
    func_8006D0F0(0x10, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_8011D510 = D_800DCEF8;
    D_8011D530 = D_800DCF00;
    D_80182D68 = (D_800DCEF8 - 1) * 0x30 - D_80139950[0];
    D_80182D78 = (D_800DCF00 - 1) * 0x30 - D_80139950[1];
    D_801B2E60 += 1;
    func_800A8864();
}
