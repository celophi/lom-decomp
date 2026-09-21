#include "common.h"

extern s32 D_801B2F50;
extern s32 D_801B2F54;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B0548(void)
{
    D_801B2F50 = 1;
    D_801B2F54 = 1;
}
