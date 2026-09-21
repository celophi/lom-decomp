#include "common.h"

extern s32 D_801B2A68;
extern s32 D_801B2A6C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008FD94(void)
{
    D_801B2A68 = 1;
    D_801B2A6C = 1;
}
