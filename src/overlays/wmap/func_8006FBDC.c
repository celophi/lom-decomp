#include "common.h"

extern s32 D_801B2400;
extern s32 D_801B2404;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006FBDC(void)
{
    D_801B2400 = 1;
    D_801B2404 = 1;
}
