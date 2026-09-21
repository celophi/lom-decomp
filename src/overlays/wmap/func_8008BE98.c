#include "common.h"

extern s32 D_801B29D0;
extern s32 D_801B29D4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008BE98(void)
{
    D_801B29D0 = 1;
    D_801B29D4 = 1;
}
