#include "common.h"

extern s32 D_801B2E08;
extern s32 D_801B2E0C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A4C88(void)
{
    D_801B2E08 = 1;
    D_801B2E0C = 1;
}
