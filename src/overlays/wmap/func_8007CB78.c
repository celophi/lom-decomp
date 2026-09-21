#include "common.h"

extern s32 D_801B2720;
extern s32 D_801B2724;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007CB78(void)
{
    D_801B2720 = 1;
    D_801B2724 = 1;
}
