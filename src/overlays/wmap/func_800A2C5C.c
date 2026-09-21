#include "common.h"

extern s32 D_801B2DB0;
extern s32 D_801B2DB4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A2C5C(void)
{
    D_801B2DB0 = 1;
    D_801B2DB4 = 1;
}
