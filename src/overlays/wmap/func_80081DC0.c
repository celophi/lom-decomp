#include "common.h"

extern s32 D_801B27F0;
extern s32 D_801B27F4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80081DC0(void)
{
    D_801B27F0 = 1;
    D_801B27F4 = 1;
}
