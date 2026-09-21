#include "common.h"

extern s32 D_801B2A58;
extern s32 D_801B2A5C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008E8E8(void)
{
    D_801B2A58 = 1;
    D_801B2A5C = 1;
}
