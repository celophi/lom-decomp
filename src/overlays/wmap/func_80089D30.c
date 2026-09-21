#include "common.h"

extern s32 D_801B2978;
extern s32 D_801B297C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80089D30(void)
{
    D_801B2978 = 1;
    D_801B297C = 1;
}
