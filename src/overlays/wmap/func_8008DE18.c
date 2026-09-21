#include "common.h"

extern s32 D_801B2A20;
extern s32 D_801B2A24;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008DE18(void)
{
    D_801B2A20 = 1;
    D_801B2A24 = 1;
}
