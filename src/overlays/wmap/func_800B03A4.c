#include "common.h"

extern s32 D_801B2F48;
extern s32 D_801B2F4C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B03A4(void)
{
    D_801B2F48 = 1;
    D_801B2F4C = 1;
}
