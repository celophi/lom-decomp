#include "common.h"

extern s32 D_801B2850;
extern s32 D_801B2854;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80083950(void)
{
    D_801B2850 = 1;
    D_801B2854 = 1;
}
