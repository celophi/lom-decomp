#include "common.h"

extern s32 D_801B2AC8;
extern s32 D_801B2ACC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800915B0(void)
{
    D_801B2AC8 = 1;
    D_801B2ACC = 1;
}
