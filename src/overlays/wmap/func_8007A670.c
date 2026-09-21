#include "common.h"

extern s32 D_801B26A8;
extern s32 D_801B26AC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007A670(void)
{
    D_801B26A8 = 1;
    D_801B26AC = 1;
}
