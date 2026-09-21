#include "common.h"

extern s32 D_801B2FD0;
extern s32 D_801B2FD4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B31F4(void)
{
    D_801B2FD0 = 1;
    D_801B2FD4 = 1;
}
