#include "common.h"

extern s32 D_801B2778;
extern s32 D_801B277C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007E9AC(void)
{
    D_801B2778 = 1;
    D_801B277C = 1;
}
