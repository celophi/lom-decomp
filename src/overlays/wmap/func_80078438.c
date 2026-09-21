#include "common.h"

extern s32 D_801B2628;
extern s32 D_801B262C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80078438(void)
{
    D_801B2628 = 1;
    D_801B262C = 1;
}
