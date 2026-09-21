#include "common.h"

extern s32 D_801B2BC8;
extern s32 D_801B2BCC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800970E0(void)
{
    D_801B2BC8 = 1;
    D_801B2BCC = 1;
}
