#include "common.h"

extern s32 D_801B2F20;
extern s32 D_801B2F24;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AFCEC(void)
{
    D_801B2F20 = 1;
    D_801B2F24 = 1;
}
