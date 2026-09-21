#include "common.h"

extern s32 D_801B2E10;
extern s32 D_801B2E14;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A4DDC(void)
{
    D_801B2E10 = 1;
    D_801B2E14 = 1;
}
