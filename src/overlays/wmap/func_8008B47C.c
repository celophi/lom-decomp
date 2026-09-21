#include "common.h"

extern s32 D_801B29B0;
extern s32 D_801B29B4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008B47C(void)
{
    D_801B29B0 = 1;
    D_801B29B4 = 1;
}
