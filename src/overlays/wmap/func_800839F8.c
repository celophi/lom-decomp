#include "common.h"

extern s32 D_801B2858;
extern s32 D_801B285C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800839F8(void)
{
    D_801B2858 = 1;
    D_801B285C = 1;
}
