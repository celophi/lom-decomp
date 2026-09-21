#include "common.h"

extern void *D_801399D4;
extern void *D_80121538;
extern u8 D_800D93F4[];
extern s32 D_80182DF0;
extern s32 D_801B25C0;
extern s32 D_801B25C4;
extern void func_800766A4(void);

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void func_80076634(void)
{
    u8 *base;

    D_801399D4 = &D_80121538;
    base = D_800D93F4;
    *(u8 *)(base + 0x6) = 0xF;
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0xE) = 0;
    *(s16 *)(base + 0x10) = -1;
    D_80182DF0 = 0;
    D_801B25C4 = 0x10;
    D_801B25C0 += 1;
    func_800766A4();
}
