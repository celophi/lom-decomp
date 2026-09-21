#include "common.h"

extern u8 D_800D9268[];
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_80121538;
extern s32 D_80182DF0;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 D_801B2B10;
extern s32 D_801B2B14;
extern void func_80092D38(void);

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0x64, 0x7C), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void func_80091A64(void)
{
    s32 i;
    u8* pa;
    u8* pb;

    D_80182DF0 = 1;
    D_800DCEA8 = 1;
    for (i = 0x64; i < 0x7C; i++)
    {
        *(s16*)(D_801AFBD0 + i * 0x14) = 0;
        pb = D_80139988 + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&D_80121538;
        pa = D_800D9268 + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 4;
    D_801B2B14 = 0x10;
    D_801B2B10 += 1;
    func_80092D38();
}
