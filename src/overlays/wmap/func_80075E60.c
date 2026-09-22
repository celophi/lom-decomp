#include "common.h"

extern void *D_801399C4;
extern void *D_8011F538;
extern u8 D_800D939C[];
extern s32 D_80182DE8;
extern s32 D_801B2598;
extern s32 D_801B259C;
extern void func_80075ED0(void);

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void func_80075E60(void)
{
    u8 *base;

    D_801399C4 = &D_8011F538;
    base = D_800D939C;
    *(u8 *)(base + 0x6) = 0xF;
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0xE) = 0;
    *(s16 *)(base + 0x10) = -1;
    D_80182DE8 = 0;
    D_801B259C = 0xC;
    D_801B2598 += 1;
    func_80075ED0();
}
