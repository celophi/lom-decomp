#include "common.h"

extern s32 D_801B27C0;
extern s32 D_801B27C4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80080514(void)
{
    D_801B27C0 = 1;
    D_801B27C4 = 1;
}
