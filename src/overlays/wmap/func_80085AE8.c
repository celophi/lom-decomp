#include "common.h"

extern s32 D_801B28B0;
extern s32 D_801B28B4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80085AE8(void)
{
    D_801B28B0 = 1;
    D_801B28B4 = 1;
}
