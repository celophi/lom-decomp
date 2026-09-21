#include "common.h"

extern s32 D_801B2C88;
extern s32 D_801B2C8C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009C5B4(void)
{
    D_801B2C88 = 1;
    D_801B2C8C = 1;
}
