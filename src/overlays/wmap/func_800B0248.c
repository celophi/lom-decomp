#include "common.h"

extern s32 D_801B2F40;
extern s32 D_801B2F44;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B0248(void)
{
    D_801B2F40 = 1;
    D_801B2F44 = 1;
}
