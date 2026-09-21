#include "common.h"

extern s32 D_801B2C54;
extern s32 D_801B2C58;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009AB98(void)
{
    D_801B2C54 = 1;
    D_801B2C58 = 1;
}
