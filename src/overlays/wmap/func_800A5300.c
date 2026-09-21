#include "common.h"

extern s32 D_801B2E20;
extern s32 D_801B2E24;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A5300(void)
{
    D_801B2E20 = 1;
    D_801B2E24 = 1;
}
