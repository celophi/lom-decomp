#include "common.h"

extern u8 D_800D9268[];
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_8011F538;
extern s32 D_80182DE4;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 D_801B2E10;
extern s32 D_801B2E14;
extern void func_800A4DF4(void);

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0xA, 0x14), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void func_800A38A0(void)
{
    s32 i;
    u8* pa;
    u8* pb;

    D_80182DE4 = 1;
    D_800DCEA8 = 1;
    for (i = 0xA; i < 0x14; i++)
    {
        *(s16*)(D_801AFBD0 + i * 0x14) = 0;
        pb = D_80139988 + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&D_8011F538;
        pa = D_800D9268 + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 6;
    D_801B2E14 = 0x10;
    D_801B2E10 += 1;
    func_800A4DF4();
}
