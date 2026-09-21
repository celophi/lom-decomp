#include "common.h"

extern s32 D_801B25E8;
extern s32 D_801B25EC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800775D8(void)
{
    D_801B25E8 = 1;
    D_801B25EC = 1;
}
