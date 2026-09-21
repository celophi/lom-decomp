#include "common.h"

extern s32 D_801B27B0;
extern s32 D_801B27B4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80080100(void)
{
    D_801B27B0 = 1;
    D_801B27B4 = 1;
}
