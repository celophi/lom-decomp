#include "common.h"

extern s32 D_801B2D58;
extern s32 D_801B2D5C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A17D8(void)
{
    D_801B2D58 = 1;
    D_801B2D5C = 1;
}
