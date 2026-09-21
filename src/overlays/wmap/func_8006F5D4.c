#include "common.h"

extern s32 D_801B2430;
extern s32 D_801B2434;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006F5D4(void)
{
    D_801B2430 = 1;
    D_801B2434 = 1;
}
