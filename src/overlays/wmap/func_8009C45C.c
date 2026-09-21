#include "common.h"

extern s32 D_801B2C80;
extern s32 D_801B2C84;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009C45C(void)
{
    D_801B2C80 = 1;
    D_801B2C84 = 1;
}
