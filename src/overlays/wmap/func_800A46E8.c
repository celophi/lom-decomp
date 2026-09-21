#include "common.h"

extern s32 D_801B2DE8;
extern s32 D_801B2DEC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A46E8(void)
{
    D_801B2DE8 = 1;
    D_801B2DEC = 1;
}
