#include "common.h"

extern s32 D_801B2D08;
extern s32 D_801B2D0C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009F498(void)
{
    D_801B2D08 = 1;
    D_801B2D0C = 1;
}
