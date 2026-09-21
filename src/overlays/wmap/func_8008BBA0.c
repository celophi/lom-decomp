#include "common.h"

extern s32 D_801B29C0;
extern s32 D_801B29C4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008BBA0(void)
{
    D_801B29C0 = 1;
    D_801B29C4 = 1;
}
