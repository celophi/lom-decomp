#include "common.h"

extern s32 D_801B2D78;
extern s32 D_801B2D7C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A2228(void)
{
    D_801B2D78 = 1;
    D_801B2D7C = 1;
}
