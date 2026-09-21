#include "common.h"

extern s32 D_801B2C40;
extern s32 D_801B2C44;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800995C4(void)
{
    D_801B2C40 = 1;
    D_801B2C44 = 1;
}
