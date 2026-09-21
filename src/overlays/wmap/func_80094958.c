#include "common.h"

extern s32 D_801B2B40;
extern s32 D_801B2B44;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80094958(void)
{
    D_801B2B40 = 1;
    D_801B2B44 = 1;
}
