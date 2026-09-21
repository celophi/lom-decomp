#include "common.h"

extern s32 D_801B28D8;
extern s32 D_801B28DC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800865BC(void)
{
    D_801B28D8 = 1;
    D_801B28DC = 1;
}
