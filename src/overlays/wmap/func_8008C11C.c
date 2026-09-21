#include "common.h"

extern s32 D_801B29E0;
extern s32 D_801B29E4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008C11C(void)
{
    D_801B29E0 = 1;
    D_801B29E4 = 1;
}
