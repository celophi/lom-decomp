#include "common.h"

extern s32 D_801B2508;
extern s32 D_801B250C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800725B0(void)
{
    D_801B2508 = 1;
    D_801B250C = 1;
}
