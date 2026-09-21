#include "common.h"

extern s32 D_801B2770;
extern s32 D_801B2774;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007E904(void)
{
    D_801B2770 = 1;
    D_801B2774 = 1;
}
