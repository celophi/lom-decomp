#include "common.h"

extern s32 D_801B2E00;
extern s32 D_801B2E04;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A4B34(void)
{
    D_801B2E00 = 1;
    D_801B2E04 = 1;
}
