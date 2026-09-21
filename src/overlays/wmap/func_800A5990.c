#include "common.h"

extern s32 D_801B2E30;
extern s32 D_801B2E34;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A5990(void)
{
    D_801B2E30 = 1;
    D_801B2E34 = 1;
}
