#include "common.h"

extern s32 D_801B25D0;
extern s32 D_801B25D4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80076B58(void)
{
    D_801B25D0 = 1;
    D_801B25D4 = 1;
}
