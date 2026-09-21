#include "common.h"

extern s32 D_801B2CC0;
extern s32 D_801B2CC4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009D2EC(void)
{
    D_801B2CC0 = 1;
    D_801B2CC4 = 1;
}
