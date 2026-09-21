#include "common.h"

extern s32 D_801B2758;
extern s32 D_801B275C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007E354(void)
{
    D_801B2758 = 1;
    D_801B275C = 1;
}
