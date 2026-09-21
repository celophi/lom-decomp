#include "common.h"

extern s32 D_801B29A0;
extern s32 D_801B29A4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008A7B0(void)
{
    D_801B29A0 = 1;
    D_801B29A4 = 1;
}
