#include "common.h"

extern s32 D_801B2F88;
extern s32 D_801B2F8C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B1550(void)
{
    D_801B2F88 = 1;
    D_801B2F8C = 1;
}
