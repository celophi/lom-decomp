#include "common.h"

extern s32 D_801B2A28;
extern s32 D_801B2A2C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008DFB8(void)
{
    D_801B2A28 = 1;
    D_801B2A2C = 1;
}
