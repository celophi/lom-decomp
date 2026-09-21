#include "common.h"

extern s32 D_801B2940;
extern s32 D_801B2944;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800883C8(void)
{
    D_801B2940 = 1;
    D_801B2944 = 1;
}
