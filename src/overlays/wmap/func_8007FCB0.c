#include "common.h"

extern s32 D_801B2798;
extern s32 D_801B279C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007FCB0(void)
{
    D_801B2798 = 1;
    D_801B279C = 1;
}
