#include "common.h"

extern s32 D_801B28E8;
extern s32 D_801B28EC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80086958(void)
{
    D_801B28E8 = 1;
    D_801B28EC = 1;
}
