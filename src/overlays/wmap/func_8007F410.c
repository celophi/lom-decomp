#include "common.h"

extern s32 D_801B2780;
extern s32 D_801B2784;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007F410(void)
{
    D_801B2780 = 1;
    D_801B2784 = 1;
}
