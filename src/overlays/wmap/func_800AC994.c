#include "common.h"

extern s32 D_801B2E88;
extern s32 D_801B2E8C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AC994(void)
{
    D_801B2E88 = 1;
    D_801B2E8C = 1;
}
