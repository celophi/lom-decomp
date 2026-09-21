#include "common.h"

extern s32 D_801B2688;
extern s32 D_801B268C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80079BAC(void)
{
    D_801B2688 = 1;
    D_801B268C = 1;
}
