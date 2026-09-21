#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2AD8;
extern s32 D_801B2ADC;
extern void func_80092CA8(void);
extern void func_8006CAC0(void (*fn)(void));
extern void func_8006683C(s32 color);

/** @brief World-map step: register a callback, set flags, advance the step. */
void func_80091FB4(void)
{
    func_8006CAC0(func_80092CA8);
    D_801ADAE0 = 1;
    func_8006683C(0x202540);
    D_801ADAF4 = 3;
    D_801B2ADC = 0x14;
    D_801B2AD8 += 1;
}
