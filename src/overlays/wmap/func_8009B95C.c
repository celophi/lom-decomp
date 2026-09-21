#include "common.h"

extern s32 D_801B2C60;
extern s32 D_801B2C64;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009B95C(void)
{
    D_801B2C60 = 1;
    D_801B2C64 = 1;
}
