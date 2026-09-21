#include "common.h"

extern s32 D_801B2C10;
extern s32 D_801B2C14;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098B68(void)
{
    D_801B2C10 = 1;
    D_801B2C14 = 1;
}
