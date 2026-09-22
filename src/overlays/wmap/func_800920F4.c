#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092490(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800920F4(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80092128(void)
{
    func_8006CAC0(func_80092490);
    D_801B2ADC = 0x26;
    D_801B2AD8 += 1;
}
