#include "common.h"

extern s32 D_801B24F0;
extern s32 D_801B24F4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800721B0(void)
{
    D_801B24F0 = 1;
    D_801B24F4 = 1;
}
