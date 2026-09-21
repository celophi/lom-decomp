#include "common.h"

extern s32 D_801B2510;
extern s32 D_801B2514;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800730C0(void)
{
    D_801B2510 = 1;
    D_801B2514 = 1;
}
