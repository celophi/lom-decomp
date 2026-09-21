#include "common.h"

extern s32 D_801B2A10;
extern s32 D_801B2A14;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008D6D4(void)
{
    D_801B2A10 = 1;
    D_801B2A14 = 1;
}
