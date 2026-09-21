#include "common.h"

extern s32 D_801B29A8;
extern s32 D_801B29AC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008B2D4(void)
{
    D_801B29A8 = 1;
    D_801B29AC = 1;
}
