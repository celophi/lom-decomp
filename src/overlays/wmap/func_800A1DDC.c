#include "common.h"

extern s32 D_801B2D60;
extern s32 D_801B2D64;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A1DDC(void)
{
    D_801B2D60 = 1;
    D_801B2D64 = 1;
}
