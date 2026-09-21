#include "common.h"

extern s32 D_801B2C98;
extern s32 D_801B2C9C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009C868(void)
{
    D_801B2C98 = 1;
    D_801B2C9C = 1;
}
