#include "common.h"

extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_8006CAC0(void (*step)(void));
extern void func_80092FE4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80092014(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80092048(void)
{
    func_8006CAC0(func_80092FE4);
    D_801B2ADC = 0x2;
    D_801B2AD8 += 1;
}
