#include "common.h"

extern s32 D_801B2EC0;
extern s32 D_801B2EC4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD190(void)
{
    D_801B2EC0 = 1;
    D_801B2EC4 = 1;
}
