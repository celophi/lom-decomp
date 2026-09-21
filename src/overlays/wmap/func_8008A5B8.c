#include "common.h"

extern s32 D_801B2998;
extern s32 D_801B299C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008A5B8(void)
{
    D_801B2998 = 1;
    D_801B299C = 1;
}
