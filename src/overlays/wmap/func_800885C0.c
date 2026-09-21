#include "common.h"

extern s32 D_801B2948;
extern s32 D_801B294C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800885C0(void)
{
    D_801B2948 = 1;
    D_801B294C = 1;
}
