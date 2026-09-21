#include "common.h"

extern s32 D_801B2DC0;
extern s32 D_801B2DC4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A2FB0(void)
{
    D_801B2DC0 = 1;
    D_801B2DC4 = 1;
}
