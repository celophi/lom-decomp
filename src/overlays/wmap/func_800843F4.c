#include "common.h"

extern s32 D_801B2868;
extern s32 D_801B286C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800843F4(void)
{
    D_801B2868 = 1;
    D_801B286C = 1;
}
