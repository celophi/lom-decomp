#include "common.h"

extern s32 D_801B2E68;
extern s32 D_801B2E6C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A8950(void)
{
    D_801B2E68 = 1;
    D_801B2E6C = 1;
}
