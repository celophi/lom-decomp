#include "common.h"

extern s32 D_801B2C00;
extern s32 D_801B2C04;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098870(void)
{
    D_801B2C00 = 1;
    D_801B2C04 = 1;
}
