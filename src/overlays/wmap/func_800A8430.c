#include "common.h"

extern s32 D_801B2E58;
extern s32 D_801B2E5C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A8430(void)
{
    D_801B2E58 = 1;
    D_801B2E5C = 1;
}
