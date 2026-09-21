#include "common.h"

extern s32 D_801B2C68;
extern s32 D_801B2C6C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009BB04(void)
{
    D_801B2C68 = 1;
    D_801B2C6C = 1;
}
