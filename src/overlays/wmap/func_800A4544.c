#include "common.h"

extern s32 D_801B2DE0;
extern s32 D_801B2DE4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A4544(void)
{
    D_801B2DE0 = 1;
    D_801B2DE4 = 1;
}
