#include "common.h"

extern s32 D_801B2410;
extern s32 D_801B2414;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006EF90(void)
{
    D_801B2410 = 1;
    D_801B2414 = 1;
}
