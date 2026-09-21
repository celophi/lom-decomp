#include "common.h"

extern s32 D_801B2D28;
extern s32 D_801B2D2C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009FC48(void)
{
    D_801B2D28 = 1;
    D_801B2D2C = 1;
}
