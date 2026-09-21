#include "common.h"

extern s32 D_801B27D8;
extern s32 D_801B27DC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80080C70(void)
{
    D_801B27D8 = 1;
    D_801B27DC = 1;
}
