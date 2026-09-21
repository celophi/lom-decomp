#include "common.h"

extern void *D_801399B4;
extern s32 D_8011D538;
extern u8 D_800D9268;
extern s32 D_801B2440;
extern s32 D_801B2444;
extern void func_8006FAC8(void);

/** @brief World-map step handler: init a UI descriptor block, set the timer, advance the step. */
void func_8006FA50(void)
{
    u8 *base = &D_800D9268;

    D_801399B4 = &D_8011D538;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEA) = 1;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    D_801B2444 = 0xC7;
    D_801B2440 += 1;
    func_8006FAC8();
}
