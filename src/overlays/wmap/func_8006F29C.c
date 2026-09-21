#include "common.h"

extern s32 D_801B2420;
extern s32 D_801B2424;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006F29C(void)
{
    D_801B2420 = 1;
    D_801B2424 = 1;
}
