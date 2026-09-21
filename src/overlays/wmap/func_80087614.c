#include "common.h"

extern s32 D_801B2908;
extern s32 D_801B290C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80087614(void)
{
    D_801B2908 = 1;
    D_801B290C = 1;
}
