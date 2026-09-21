#include "common.h"

extern s32 D_801B2F68;
extern s32 D_801B2F6C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B0BEC(void)
{
    D_801B2F68 = 1;
    D_801B2F6C = 1;
}
