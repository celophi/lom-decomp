#include "common.h"

extern s32 D_801B2598;
extern s32 D_801B259C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80075E48(void)
{
    D_801B2598 = 1;
    D_801B259C = 1;
}
