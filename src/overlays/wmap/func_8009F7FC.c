#include "common.h"

extern s32 D_801B2D18;
extern s32 D_801B2D1C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009F7FC(void)
{
    D_801B2D18 = 1;
    D_801B2D1C = 1;
}
