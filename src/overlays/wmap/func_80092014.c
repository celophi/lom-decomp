#include "common.h"

extern s32 D_801B2ADC;
extern s32 D_801B2AD8;

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
