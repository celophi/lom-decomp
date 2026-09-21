#include "common.h"

extern s32 D_801B2C38;
extern s32 D_801B2C3C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009933C(void)
{
    D_801B2C38 = 1;
    D_801B2C3C = 1;
}
