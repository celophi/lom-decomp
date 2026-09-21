#include "common.h"

extern s32 D_801B2950;
extern s32 D_801B2954;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800887B8(void)
{
    D_801B2950 = 1;
    D_801B2954 = 1;
}
