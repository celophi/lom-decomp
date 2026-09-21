#include "common.h"

extern s32 D_801B2C18;
extern s32 D_801B2C1C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098CC0(void)
{
    D_801B2C18 = 1;
    D_801B2C1C = 1;
}
