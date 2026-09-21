#include "common.h"

extern s32 D_801B28E0;
extern s32 D_801B28E4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80086760(void)
{
    D_801B28E0 = 1;
    D_801B28E4 = 1;
}
