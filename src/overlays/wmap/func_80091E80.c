#include "common.h"

extern s32 D_801B2AD8;
extern s32 D_801B2ADC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80091E80(void)
{
    D_801B2AD8 = 1;
    D_801B2ADC = 1;
}
