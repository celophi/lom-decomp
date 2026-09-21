#include "common.h"

extern s32 D_801B2AD0;
extern s32 D_801B2AD4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80091C84(void)
{
    D_801B2AD0 = 1;
    D_801B2AD4 = 1;
}
