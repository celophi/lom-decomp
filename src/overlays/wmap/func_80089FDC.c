#include "common.h"

extern s32 D_801B2988;
extern s32 D_801B298C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80089FDC(void)
{
    D_801B2988 = 1;
    D_801B298C = 1;
}
