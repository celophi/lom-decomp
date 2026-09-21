#include "common.h"

extern s32 D_801B2C78;
extern s32 D_801B2C7C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009C2B8(void)
{
    D_801B2C78 = 1;
    D_801B2C7C = 1;
}
