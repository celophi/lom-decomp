#include "common.h"

extern s32 D_801B2F00;
extern s32 D_801B2F04;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AF62C(void)
{
    D_801B2F00 = 1;
    D_801B2F04 = 1;
}
