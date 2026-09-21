#include "common.h"

extern s32 D_801B2890;
extern s32 D_801B2894;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80084DA8(void)
{
    D_801B2890 = 1;
    D_801B2894 = 1;
}
