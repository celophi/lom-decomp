#include "common.h"

extern s32 D_801B2A00;
extern s32 D_801B2A04;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008C66C(void)
{
    D_801B2A00 = 1;
    D_801B2A04 = 1;
}
