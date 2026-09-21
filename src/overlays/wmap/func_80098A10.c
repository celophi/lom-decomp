#include "common.h"

extern s32 D_801B2C08;
extern s32 D_801B2C0C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098A10(void)
{
    D_801B2C08 = 1;
    D_801B2C0C = 1;
}
