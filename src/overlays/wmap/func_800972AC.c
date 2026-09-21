#include "common.h"

extern s32 D_801B2BD0;
extern s32 D_801B2BD4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800972AC(void)
{
    D_801B2BD0 = 1;
    D_801B2BD4 = 1;
}
