#include "common.h"

extern s32 D_801B2970;
extern s32 D_801B2974;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80089B90(void)
{
    D_801B2970 = 1;
    D_801B2974 = 1;
}
