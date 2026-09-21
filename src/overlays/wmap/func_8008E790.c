#include "common.h"

extern s32 D_801B2A50;
extern s32 D_801B2A54;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008E790(void)
{
    D_801B2A50 = 1;
    D_801B2A54 = 1;
}
