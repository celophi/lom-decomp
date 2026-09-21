#include "common.h"

extern s32 D_801B2BB8;
extern s32 D_801B2BBC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80096BB8(void)
{
    D_801B2BB8 = 1;
    D_801B2BBC = 1;
}
