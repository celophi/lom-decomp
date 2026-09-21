#include "common.h"

extern s32 D_801B2E38;
extern s32 D_801B2E3C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A5CD8(void)
{
    D_801B2E38 = 1;
    D_801B2E3C = 1;
}
