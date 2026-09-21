#include "common.h"

extern s32 D_801B2F70;
extern s32 D_801B2F74;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B0D9C(void)
{
    D_801B2F70 = 1;
    D_801B2F74 = 1;
}
