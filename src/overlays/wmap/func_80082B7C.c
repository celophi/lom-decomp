#include "common.h"

extern s32 D_801B2820;
extern s32 D_801B2824;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80082B7C(void)
{
    D_801B2820 = 1;
    D_801B2824 = 1;
}
