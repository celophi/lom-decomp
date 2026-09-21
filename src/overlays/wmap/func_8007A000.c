#include "common.h"

extern s32 D_801B2690;
extern s32 D_801B2694;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007A000(void)
{
    D_801B2690 = 1;
    D_801B2694 = 1;
}
