#include "common.h"

extern s32 D_801B2EE0;
extern s32 D_801B2EE4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD688(void)
{
    D_801B2EE0 = 1;
    D_801B2EE4 = 1;
}
