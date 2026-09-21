#include "common.h"

extern s32 D_801B2EA0;
extern s32 D_801B2EA4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACD00(void)
{
    D_801B2EA0 = 1;
    D_801B2EA4 = 1;
}
