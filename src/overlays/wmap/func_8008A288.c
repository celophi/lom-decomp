#include "common.h"

extern s32 D_801B2990;
extern s32 D_801B2994;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008A288(void)
{
    D_801B2990 = 1;
    D_801B2994 = 1;
}
