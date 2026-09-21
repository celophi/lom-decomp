#include "common.h"

extern s32 D_801B2648;
extern s32 D_801B264C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80078C9C(void)
{
    D_801B2648 = 1;
    D_801B264C = 1;
}
