#include "common.h"

extern s32 D_801B2928;
extern s32 D_801B292C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80087F48(void)
{
    D_801B2928 = 1;
    D_801B292C = 1;
}
