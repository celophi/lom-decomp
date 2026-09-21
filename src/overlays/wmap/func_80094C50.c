#include "common.h"

extern s32 D_801B2B50;
extern s32 D_801B2B54;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80094C50(void)
{
    D_801B2B50 = 1;
    D_801B2B54 = 1;
}
