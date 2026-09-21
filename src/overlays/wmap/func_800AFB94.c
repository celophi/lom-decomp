#include "common.h"

extern s32 D_801B2F18;
extern s32 D_801B2F1C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AFB94(void)
{
    D_801B2F18 = 1;
    D_801B2F1C = 1;
}
