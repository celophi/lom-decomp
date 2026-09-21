#include "common.h"

extern s32 D_801B2A38;
extern s32 D_801B2A3C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008E2E0(void)
{
    D_801B2A38 = 1;
    D_801B2A3C = 1;
}
