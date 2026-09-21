#include "common.h"

extern s32 D_801B2AC0;
extern s32 D_801B2AC4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80091508(void)
{
    D_801B2AC0 = 1;
    D_801B2AC4 = 1;
}
