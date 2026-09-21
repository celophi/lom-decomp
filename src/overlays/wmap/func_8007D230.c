#include "common.h"

extern s32 D_801B2738;
extern s32 D_801B273C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007D230(void)
{
    D_801B2738 = 1;
    D_801B273C = 1;
}
