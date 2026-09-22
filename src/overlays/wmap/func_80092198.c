#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAF4;
extern s32 D_801B2AD8;
extern s32 D_801B2ADC;
extern void func_800929F0(void);
extern void func_8006683C(s32 arg);

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_80092198(void)
{
    func_8006CAC0(func_800929F0);
    func_8006683C(0x404050);
    D_801ADAF4 = 8;
    D_801B2ADC = 4;
    D_801B2AD8 += 1;
}
