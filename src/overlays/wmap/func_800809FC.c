#include "common.h"

extern s32 D_801B27D0;
extern s32 D_801B27D4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800809FC(void)
{
    D_801B27D0 = 1;
    D_801B27D4 = 1;
}
