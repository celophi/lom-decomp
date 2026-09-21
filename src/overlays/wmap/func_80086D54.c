#include "common.h"

extern s32 D_801B2900;
extern s32 D_801B2904;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80086D54(void)
{
    D_801B2900 = 1;
    D_801B2904 = 1;
}
