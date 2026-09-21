#include "common.h"

extern s32 D_801B2B60;
extern s32 D_801B2B64;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80094EF8(void)
{
    D_801B2B60 = 1;
    D_801B2B64 = 1;
}
