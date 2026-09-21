#include "common.h"

extern s32 D_801B2828;
extern s32 D_801B282C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80082DE8(void)
{
    D_801B2828 = 1;
    D_801B282C = 1;
}
