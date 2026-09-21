#include "common.h"

extern s32 D_801B2BF0;
extern s32 D_801B2BF4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098460(void)
{
    D_801B2BF0 = 1;
    D_801B2BF4 = 1;
}
