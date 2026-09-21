#include "common.h"

extern s32 D_801B25C0;
extern s32 D_801B25C4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007661C(void)
{
    D_801B25C0 = 1;
    D_801B25C4 = 1;
}
