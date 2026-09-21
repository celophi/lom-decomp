#include "common.h"

extern s32 D_801B2838;
extern s32 D_801B283C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800832C0(void)
{
    D_801B2838 = 1;
    D_801B283C = 1;
}
