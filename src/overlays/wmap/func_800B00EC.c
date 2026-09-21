#include "common.h"

extern s32 D_801B2F38;
extern s32 D_801B2F3C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B00EC(void)
{
    D_801B2F38 = 1;
    D_801B2F3C = 1;
}
