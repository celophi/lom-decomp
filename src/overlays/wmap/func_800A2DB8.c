#include "common.h"

extern s32 D_801B2DB8;
extern s32 D_801B2DBC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A2DB8(void)
{
    D_801B2DB8 = 1;
    D_801B2DBC = 1;
}
