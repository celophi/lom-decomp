#include "common.h"

extern s32 D_801B28F8;
extern s32 D_801B28FC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80086C08(void)
{
    D_801B28F8 = 1;
    D_801B28FC = 1;
}
