#include "common.h"

extern s32 D_801B28A8;
extern s32 D_801B28AC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800852E8(void)
{
    D_801B28A8 = 1;
    D_801B28AC = 1;
}
