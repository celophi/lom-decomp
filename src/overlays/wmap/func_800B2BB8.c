#include "common.h"

extern s32 D_801B2FB0;
extern s32 D_801B2FB4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2BB8(void)
{
    D_801B2FB0 = 1;
    D_801B2FB4 = 1;
}
