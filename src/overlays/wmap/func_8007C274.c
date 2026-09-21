#include "common.h"

extern s32 D_801B26F8;
extern s32 D_801B26FC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007C274(void)
{
    D_801B26F8 = 1;
    D_801B26FC = 1;
}
