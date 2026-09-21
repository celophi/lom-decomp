#include "common.h"

extern s32 D_801B2C70;
extern s32 D_801B2C74;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009C114(void)
{
    D_801B2C70 = 1;
    D_801B2C74 = 1;
}
