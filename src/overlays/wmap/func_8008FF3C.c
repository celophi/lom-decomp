#include "common.h"

extern s32 D_801B2A70;
extern s32 D_801B2A74;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008FF3C(void)
{
    D_801B2A70 = 1;
    D_801B2A74 = 1;
}
