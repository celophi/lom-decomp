#include "common.h"

extern s32 D_801B2F08;
extern s32 D_801B2F0C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AF89C(void)
{
    D_801B2F08 = 1;
    D_801B2F0C = 1;
}
