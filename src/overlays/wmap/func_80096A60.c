#include "common.h"

extern s32 D_801B2BB0;
extern s32 D_801B2BB4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80096A60(void)
{
    D_801B2BB0 = 1;
    D_801B2BB4 = 1;
}
