#include "common.h"

extern s32 D_801B2FD8;
extern s32 D_801B2FDC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B3350(void)
{
    D_801B2FD8 = 1;
    D_801B2FDC = 1;
}
