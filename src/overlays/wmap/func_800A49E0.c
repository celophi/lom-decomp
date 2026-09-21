#include "common.h"

extern s32 D_801B2DF8;
extern s32 D_801B2DFC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A49E0(void)
{
    D_801B2DF8 = 1;
    D_801B2DFC = 1;
}
