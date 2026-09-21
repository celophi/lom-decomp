#include "common.h"

extern s32 D_801B2630;
extern s32 D_801B2634;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007889C(void)
{
    D_801B2630 = 1;
    D_801B2634 = 1;
}
