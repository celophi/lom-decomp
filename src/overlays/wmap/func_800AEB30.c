#include "common.h"

extern s32 D_801B2EF0;
extern s32 D_801B2EF4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AEB30(void)
{
    D_801B2EF0 = 1;
    D_801B2EF4 = 1;
}
