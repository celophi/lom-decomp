#include "common.h"

extern s32 D_801B2550;
extern s32 D_801B2554;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80073FB8(void)
{
    D_801B2550 = 1;
    D_801B2554 = 1;
}
