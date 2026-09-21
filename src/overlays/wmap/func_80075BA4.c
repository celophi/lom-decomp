#include "common.h"

extern s32 D_801B2590;
extern s32 D_801B2594;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80075BA4(void)
{
    D_801B2590 = 1;
    D_801B2594 = 1;
}
