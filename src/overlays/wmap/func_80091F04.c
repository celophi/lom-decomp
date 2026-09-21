#include "common.h"

extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_8006CAC0(void (*step)(void));
extern void func_80092898(void);
extern void func_800922F4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80091F04(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80091F38(void)
{
    func_8006CAC0(func_80092898);
    func_8006CAC0(func_800922F4);
    D_801B2ADC = 0x4;
    D_801B2AD8 += 1;
}
