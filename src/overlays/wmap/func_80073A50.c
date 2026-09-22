#include "common.h"

extern void *D_801399B4;
extern s32 D_8011D538;
extern u8 D_800D9268;
extern s32 D_801B2530;
extern s32 D_801B2534;
extern void func_8007287C(void);

/** @brief World-map step handler: init substate block and advance. */
void func_80073A50(void)
{
    u8 *base = &D_800D9268;

    D_801399B4 = &D_8011D538;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xEA) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    D_801B2534 = 0x32;
    D_801B2530 += 1;
    func_8007287C();
}
