#include "common.h"

extern s32 D_801B2AA0;
extern s32 D_801B2AA4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090F5C(void)
{
    D_801B2AA0 = 1;
    D_801B2AA4 = 1;
}
