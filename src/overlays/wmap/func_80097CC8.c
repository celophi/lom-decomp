#include "common.h"

extern s32 D_801B2BE0;
extern s32 D_801B2BE4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80097CC8(void)
{
    D_801B2BE0 = 1;
    D_801B2BE4 = 1;
}
