#include "common.h"

extern s32 D_801B2D38;
extern s32 D_801B2D3C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A013C(void)
{
    D_801B2D38 = 1;
    D_801B2D3C = 1;
}
