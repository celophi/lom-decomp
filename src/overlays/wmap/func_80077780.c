#include "common.h"

extern s32 D_801B25F0;
extern s32 D_801B25F4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80077780(void)
{
    D_801B25F0 = 1;
    D_801B25F4 = 1;
}
