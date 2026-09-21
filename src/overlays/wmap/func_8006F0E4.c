#include "common.h"

extern s32 D_801B2418;
extern s32 D_801B241C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006F0E4(void)
{
    D_801B2418 = 1;
    D_801B241C = 1;
}
