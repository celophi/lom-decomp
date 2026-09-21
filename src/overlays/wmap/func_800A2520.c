#include "common.h"

extern s32 D_801B2D88;
extern s32 D_801B2D8C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A2520(void)
{
    D_801B2D88 = 1;
    D_801B2D8C = 1;
}
