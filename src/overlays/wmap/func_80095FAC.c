#include "common.h"

extern s32 D_801B2B88;
extern s32 D_801B2B8C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80095FAC(void)
{
    D_801B2B88 = 1;
    D_801B2B8C = 1;
}
