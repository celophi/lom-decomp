#include "common.h"

extern s32 D_801B2C90;
extern s32 D_801B2C94;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009C70C(void)
{
    D_801B2C90 = 1;
    D_801B2C94 = 1;
}
