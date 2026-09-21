#include "common.h"

extern s32 D_801B25F8;
extern s32 D_801B25FC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80077CA4(void)
{
    D_801B25F8 = 1;
    D_801B25FC = 1;
}
