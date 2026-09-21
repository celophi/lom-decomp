#include "common.h"

extern s32 D_801B2760;
extern s32 D_801B2764;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007E4F8(void)
{
    D_801B2760 = 1;
    D_801B2764 = 1;
}
