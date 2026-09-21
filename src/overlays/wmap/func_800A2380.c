#include "common.h"

extern s32 D_801B2D80;
extern s32 D_801B2D84;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A2380(void)
{
    D_801B2D80 = 1;
    D_801B2D84 = 1;
}
