#include "common.h"

extern s32 D_801B2B38;
extern s32 D_801B2B3C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800946EC(void)
{
    D_801B2B38 = 1;
    D_801B2B3C = 1;
}
