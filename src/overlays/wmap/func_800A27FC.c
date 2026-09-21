#include "common.h"

extern s32 D_801B2D98;
extern s32 D_801B2D9C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A27FC(void)
{
    D_801B2D98 = 1;
    D_801B2D9C = 1;
}
