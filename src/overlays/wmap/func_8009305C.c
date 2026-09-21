#include "common.h"

extern s32 D_801B2B18;
extern s32 D_801B2B1C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009305C(void)
{
    D_801B2B18 = 1;
    D_801B2B1C = 1;
}
