#include "common.h"

extern s32 D_801B2C20;
extern s32 D_801B2C24;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098EA0(void)
{
    D_801B2C20 = 1;
    D_801B2C24 = 1;
}
