#include "common.h"

extern s32 D_801B2740;
extern s32 D_801B2744;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007DC8C(void)
{
    D_801B2740 = 1;
    D_801B2744 = 1;
}
