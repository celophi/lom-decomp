#include "common.h"

extern s32 D_801B2C28;
extern s32 D_801B2C2C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80099040(void)
{
    D_801B2C28 = 1;
    D_801B2C2C = 1;
}
