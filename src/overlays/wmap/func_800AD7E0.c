#include "common.h"

extern s32 D_801B2EE8;
extern s32 D_801B2EEC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD7E0(void)
{
    D_801B2EE8 = 1;
    D_801B2EEC = 1;
}
