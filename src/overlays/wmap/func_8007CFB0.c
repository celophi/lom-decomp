#include "common.h"

extern s32 D_801B2730;
extern s32 D_801B2734;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007CFB0(void)
{
    D_801B2730 = 1;
    D_801B2734 = 1;
}
