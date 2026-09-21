#include "common.h"

extern s32 D_801B2680;
extern s32 D_801B2684;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80079A04(void)
{
    D_801B2680 = 1;
    D_801B2684 = 1;
}
