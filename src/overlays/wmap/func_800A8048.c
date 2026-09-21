#include "common.h"

extern s32 D_801B2E50;
extern s32 D_801B2E54;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A8048(void)
{
    D_801B2E50 = 1;
    D_801B2E54 = 1;
}
