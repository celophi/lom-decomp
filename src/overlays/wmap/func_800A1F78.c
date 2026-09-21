#include "common.h"

extern s32 D_801B2D68;
extern s32 D_801B2D6C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A1F78(void)
{
    D_801B2D68 = 1;
    D_801B2D6C = 1;
}
