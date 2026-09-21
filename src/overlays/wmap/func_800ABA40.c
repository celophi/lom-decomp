#include "common.h"

extern s32 D_801B2E80;
extern s32 D_801B2E84;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ABA40(void)
{
    D_801B2E80 = 1;
    D_801B2E84 = 1;
}
