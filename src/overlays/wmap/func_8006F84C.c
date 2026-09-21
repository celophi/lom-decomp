#include "common.h"

extern s32 D_801B2438;
extern s32 D_801B243C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006F84C(void)
{
    D_801B2438 = 1;
    D_801B243C = 1;
}
