#include "common.h"

extern s32 D_801B2FB8;
extern s32 D_801B2FBC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2D60(void)
{
    D_801B2FB8 = 1;
    D_801B2FBC = 1;
}
