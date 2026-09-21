#include "common.h"

extern s32 D_801B29F0;
extern s32 D_801B29F4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008C414(void)
{
    D_801B29F0 = 1;
    D_801B29F4 = 1;
}
