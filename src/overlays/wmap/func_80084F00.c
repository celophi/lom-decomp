#include "common.h"

extern s32 D_801B2898;
extern s32 D_801B289C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80084F00(void)
{
    D_801B2898 = 1;
    D_801B289C = 1;
}
