#include "common.h"

extern s32 D_801B2E18;
extern s32 D_801B2E1C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A5100(void)
{
    D_801B2E18 = 1;
    D_801B2E1C = 1;
}
