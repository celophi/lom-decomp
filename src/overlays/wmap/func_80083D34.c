#include "common.h"

extern s32 D_801B2860;
extern s32 D_801B2864;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80083D34(void)
{
    D_801B2860 = 1;
    D_801B2864 = 1;
}
